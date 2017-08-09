// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

package com.microsoft.bond.protocol;

import com.microsoft.bond.BondDataType;
import com.microsoft.bond.BondEnum;
import com.microsoft.bond.ProtocolType;

import java.io.IOException;
import java.io.OutputStream;

/**
 * Implements Simple Binary serialization.
 * Refer to {@see https://microsoft.github.io/bond/reference/cpp/simple__binary_8h_source.html} for details.
 */
public final class SimpleBinaryWriter implements ProtocolWriter {

    // Indicates the protocol type when marshalling.
    private static final ProtocolType MAGIC = ProtocolType.SIMPLE_PROTOCOL;

    private final short protocolVersion;
    private final BinaryStreamWriter writer;

    public SimpleBinaryWriter(final OutputStream outputStream, final short protocolVersion) {
        if (outputStream == null) {
            throw new IllegalArgumentException("Argument stream must not be null");
        }

        if (protocolVersion != 1 && protocolVersion != 2) {
            throw new IllegalArgumentException("Invalid protocol version: " + protocolVersion);
        }

        this.writer = new BinaryStreamWriter(outputStream);
        this.protocolVersion = protocolVersion;
    }

    @Override
    public boolean usesMarshaledBonded() {
        return true;
    }

    @Override
    public void writeVersion() throws IOException {
        this.writer.writeInt16((short) MAGIC.value);
        this.writer.writeInt16(this.protocolVersion);
    }

    @Override
    public void writeFieldOmitted(
            final BondDataType type, final int id, final FieldMetadata metadata) throws IOException {
        assert !metadata.isDefaultNothing();

        switch (type.value) {
            case BondDataType.Values.BT_BOOL:
                writeBool((Boolean) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_INT8:
                writeInt8((Byte) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_INT16:
                writeInt16((Short) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_INT32:
                // Could be an int32, could be an enum.
                final Object def = metadata.getDefaultValue();
                if (def instanceof BondEnum) {
                    writeInt32(((BondEnum) def).getValue());
                } else {
                    writeInt32((Integer) def);
                }
                break;
            case BondDataType.Values.BT_INT64:
                writeInt64((Long) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_UINT8:
                writeInt8((Byte) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_UINT16:
                writeInt16((Short) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_UINT32:
                writeInt32((Integer) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_UINT64:
                writeInt64((Long) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_FLOAT:
                writeFloat((Float) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_DOUBLE:
                writeDouble((Double) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_STRING:
                writeString((String) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_WSTRING:
                writeWString((String) metadata.getDefaultValue());
                break;
            case BondDataType.Values.BT_LIST:
            case BondDataType.Values.BT_SET:
            case BondDataType.Values.BT_MAP:
                writeContainerBegin(0, type);
                break;
            default:
                throw new IllegalArgumentException("Invalid bondType " + type.toString());
        }
    }

    private void writeLength(final int count) throws IOException {
        if (this.protocolVersion == 2) {
            this.writer.writeVarUInt32(count);
        } else {
            this.writer.writeInt32(count);
        }
    }

    @Override
    public void writeContainerBegin(final int count, final BondDataType elementType)
            throws IOException {
        writeLength(count);
    }

    @Override
    public void writeContainerBegin(final int count, final BondDataType keyType, final BondDataType valueType)
            throws IOException {
        writeLength(count);
    }

    @Override
    public void writeContainerEnd() throws IOException {
    }

    @Override
    public void writeInt8(final byte value) throws IOException {
        this.writer.writeInt8(value);
    }

    @Override
    public void writeInt16(final short value) throws IOException {
        this.writer.writeInt16(value);
    }

    @Override
    public void writeInt32(final int value) throws IOException {
        this.writer.writeInt32(value);
    }

    @Override
    public void writeInt64(final long value) throws IOException {
        this.writer.writeInt64(value);
    }

    @Override
    public void writeUInt8(final byte value) throws IOException {
        // reinterpret the sign bit as the high-order bit
        this.writer.writeInt8(value);
    }

    @Override
    public void writeUInt16(final short value) throws IOException {
        // reinterpret the sign bit as the high-order bit
        this.writer.writeInt16(value);
    }

    @Override
    public void writeUInt32(final int value) throws IOException {
        // reinterpret the sign bit as the high-order bit
        this.writer.writeInt32(value);
    }

    @Override
    public void writeUInt64(final long value) throws IOException {
        // reinterpret the sign bit as the high-order bit
        this.writer.writeInt64(value);
    }

    @Override
    public void writeFloat(final float value) throws IOException {
        this.writer.writeFloat(value);
    }

    @Override
    public void writeDouble(final double value) throws IOException {
        this.writer.writeDouble(value);
    }

    @Override
    public void writeBytes(final byte[] value) throws IOException {
        this.writer.writeBytes(value);
    }

    @Override
    public void writeBool(final boolean value) throws IOException {
        this.writer.writeBool(value);
    }

    @Override
    public void writeString(final String value) throws IOException {
        if (value.isEmpty()) {
            // avoid calling encoder for an empty string
            writeLength(0);
        } else {
            final byte[] bytes = StringHelper.encodeString(value);
            writeLength(bytes.length);
            this.writer.writeBytes(bytes);
        }
    }

    @Override
    public void writeWString(final String value) throws IOException {
        if (value.isEmpty()) {
            // avoid calling encoder for an empty string
            writeLength(0);
        } else {
            final byte[] bytes = StringHelper.encodeWString(value);
            writeLength(bytes.length / 2);
            this.writer.writeBytes(bytes);
        }
    }

    //
    // The remaining ProtocolWriter methods are unused in untagged protocols.
    //

    @Override
    public void writeStructBegin(final StructMetadata metadata) throws IOException {
    }

    @Override
    public void writeStructEnd() throws IOException {
    }

    @Override
    public void writeBaseBegin(final StructMetadata metadata) throws IOException {
    }

    @Override
    public void writeBaseEnd() throws IOException {
    }

    @Override
    public void writeFieldBegin(
        final BondDataType type, final int id, final FieldMetadata metadata) throws IOException {
    }

    @Override
    public void writeFieldEnd() throws IOException {
    }
}
