"""Find referenced containers from Git commits."""

import logging
import re
import subprocess
from typing import AbstractSet, Iterable, NewType, Sequence

from .config import REGISTRY_NAME, REPOSITORY_NAME

logger = logging.getLogger(__name__)

_RevListRoots = NewType('RevListRoots', Sequence[Sequence[str]]) # pylint: disable=invalid-name
_BlobId = NewType('BlobId', str) # pylint: disable=invalid-name

"""A type for the full name of an image. E.g.,
"bondciimages.azurecr.io/ubuntu-1604:build-12345".
"""
ImageName = NewType('ImageName', str) # pylint: disable=invalid-name

"""A type for the an image tag. E.g., "build-12345"."""
ImageTag = NewType('ImageTag', str) # pylint: disable=invalid-name

def _blobs_from_roots(
        repo_path: str,
        roots: _RevListRoots) -> AbstractSet[_BlobId]:
    """Return the blob IDs of the revisions of .travis.yml for all the commits
specified by the given `roots`.

    :param repo_path: Path to the repository to inspect.

    :param roots: A collection of argument lists to pass to ``git
    rev-list`` to limit the matched commits.

    :return The blob IDs of the .travis.yml revisions.
    """

    commit_ids = set()
    for root_args in roots:
        git_rev_list_cmd_line = ['git', '-C', repo_path, 'rev-list']
        git_rev_list_cmd_line.extend(root_args)

        logger.debug('Invoking %s', git_rev_list_cmd_line)

        rev_list_output = subprocess.check_output(git_rev_list_cmd_line, stderr=subprocess.PIPE)
        commit_ids.update(str(rev_list_output, 'utf-8').splitlines())
        logger.debug('Commit IDs currently: %s', commit_ids)

    blob_ids = set()
    for commit in commit_ids:
        git_ls_tree_cmd_line = ['git',
                                '-C', repo_path,
                                'ls-tree', commit,
                                '--', ':/.travis.yml']
        logger.debug('Invoking %s', git_ls_tree_cmd_line)
        ls_tree_output = subprocess.check_output(git_ls_tree_cmd_line, stderr=subprocess.PIPE)

        for line in str(ls_tree_output, 'utf-8').splitlines():
            logger.debug('Parsing ls-tree output: "%s"', line)

            # Expecing output like:
            #
            # 100644 blob cbe47c031fb164dca034aac640de08cf35170a68	.travis.yml
            parts = line.split()

            # The length of parts may not be exactly 4 if the name
            # contains a space. (Though, this shouldn't happen in this
            # particular case.) Instead of parsing exactly, we check
            # that we have at least the first three parts and call it
            # good enough.
            if len(parts) < 3:
                raise ValueError('Cannot parse ls-tree output: "{}"'.format(line))

            obj_type = parts[1]
            blob_id = parts[2]
            if obj_type == 'blob':
                blob_ids.add(blob_id)

    logger.debug('Blobs: %s', blob_ids)
    return frozenset(blob_ids)

def _images_from_blobs(
        repo_path: str,
        commits: Iterable[_BlobId]) -> AbstractSet[ImageName]:
    """Get the full names (including tags) of the images used for the
CI build in the given blobs.

    :param repo_path: Path to the repository to inspect.

    :param commits: The Git blob IDs of files in which to look for
    containers. It's assumed that these are blob IDs of .travis.yml or
    similar files.

    :return The full container names used by the specificed blobs.

    Containers are found by looking for a line like

    CI_BUILD_CONTAINER=...
    """

    container_names = set()

    for commit in commits:
        git_show_cmd_line = ['git', '-C', repo_path, 'show', commit]
        logger.debug('Invoking %s', git_show_cmd_line)
        git_show_output = subprocess.check_output(git_show_cmd_line, stderr=subprocess.PIPE)

        matches = re.finditer(b'^.+CI_BUILD_CONTAINER=(.+)\\s*$',
                              git_show_output,
                              re.MULTILINE)
        container_names.update(map((lambda match: str(match.group(1), 'utf-8')),
                                   matches))
        logger.debug('Container names currently: %s', container_names)

    return frozenset(container_names)

def live_images(
        repo_path: str,
        roots: _RevListRoots) -> AbstractSet[ImageName]:
    """Return the image names used by all of the commits specified by the
given `roots`.

    :param repo_path: Path to the repository to inspect.

    :param roots: A collection of argument lists to pass to ``git
    rev-list`` to limit the matched commits.
    """
    return _images_from_blobs(repo_path,
                              _blobs_from_roots(repo_path, roots))

def live_tags(repo_path: str,
              roots: _RevListRoots) -> AbstractSet[ImageTag]:
    """Return the image tags that are referenced by .travis.yml files in
the commits specified by the given `roots`.

    :param repo_path: Path to the repository to inspect.

    :param roots: A collection of argument lists to pass to ``git rev-list``
    to limit the matched commits.
    """
    expected_prefix = '{}.azurecr.io/{}'.format(REGISTRY_NAME, REPOSITORY_NAME)
    def matches_expected_prefix(image_name: str) -> bool:
        """Check (and log) whether an image name is from the expected repository."""
        if image_name.startswith(expected_prefix):
            return True

        logger.info(
            'Discarding image "%s" that does not match expected prefix "%s"',
            image_name,
            expected_prefix)
        return False

    return frozenset(map((lambda image_name: image_name.split(':')[1]),
                         filter(matches_expected_prefix, live_images(repo_path, roots))))
