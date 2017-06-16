package com.microsoft.bond.gradle

import org.gradle.api.Plugin
import org.gradle.api.Project

class BondPlugin implements Plugin<Project> {
    @Override
    void apply(Project project) {
        project.tasks.create("bondCodegen", BondCodegen)
    }
}
