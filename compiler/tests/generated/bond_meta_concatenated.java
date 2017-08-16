
package deprecated.bondmeta;

@javax.annotation.Generated("gbc")
public class HasMetaFields implements com.microsoft.bond.BondSerializable {
    
    private static final class StructBondTypeImpl extends com.microsoft.bond.StructBondType<HasMetaFields> {
        
        static final class StructBondTypeBuilderImpl extends com.microsoft.bond.StructBondType.StructBondTypeBuilder<HasMetaFields> {
            
            @Override
            public final int getGenericTypeParameterCount() {
                return 0;
            }

            @Override
            protected final com.microsoft.bond.StructBondType<HasMetaFields> buildNewInstance(com.microsoft.bond.BondType[] genericTypeArguments) {
                return new StructBondTypeImpl(null);
            }

            static void register() {
                registerStructType(HasMetaFields.class, new StructBondTypeBuilderImpl());
            }
        }

        private com.microsoft.bond.StructBondType.StringStructField full_name;

        private com.microsoft.bond.StructBondType.StringStructField name;

        private StructBondTypeImpl(com.microsoft.bond.GenericTypeSpecialization genericTypeSpecialization) {
            super(genericTypeSpecialization);
        }
        
        @Override
        protected final void initialize() {
            this.full_name = new com.microsoft.bond.StructBondType.StringStructField(this, 0, "full_name", com.microsoft.bond.Modifier.Optional);
            this.name = new com.microsoft.bond.StructBondType.StringStructField(this, 1, "name", com.microsoft.bond.Modifier.Optional);
            super.initializeBaseAndFields(null, this.full_name, this.name);
        }

        @Override
        public final String getName() {
            return "HasMetaFields";
        }

        @Override
        public final String getQualifiedName() {
            return "deprecated.bondmeta.HasMetaFields";
        }

        @Override
        public final java.lang.Class<HasMetaFields> getValueClass() {
            return (java.lang.Class<HasMetaFields>) (java.lang.Class) HasMetaFields.class;
        }

        @Override
        public final HasMetaFields newInstance() {
            return new HasMetaFields();
        }
        
        @Override
        protected final void serializeStructFields(com.microsoft.bond.BondType.SerializationContext context, HasMetaFields value) throws java.io.IOException {
            this.full_name.serialize(context, value.full_name);
            this.name.serialize(context, value.name);
        }
        
        @Override
        protected final void deserializeStructFields(com.microsoft.bond.BondType.TaggedDeserializationContext context, HasMetaFields value) throws java.io.IOException {
            boolean __has_full_name = false;
            boolean __has_name = false;
            while (this.readField(context)) {
                switch (context.readFieldResult.id) {
                    case 0:
                        value.full_name = this.full_name.deserialize(context, __has_full_name);
                        __has_full_name = true;
                        break;
                    case 1:
                        value.name = this.name.deserialize(context, __has_name);
                        __has_name = true;
                        break;
                    default:
                        context.reader.skip(context.readFieldResult.type);
                        break;
                }
            }
            this.full_name.verifyDeserialized(__has_full_name);
            this.name.verifyDeserialized(__has_name);
        }
        
        @Override
        protected final void deserializeStructFields(com.microsoft.bond.BondType.UntaggedDeserializationContext context, HasMetaFields value) throws java.io.IOException {
            value.full_name = this.full_name.deserialize(context);
            value.name = this.name.deserialize(context);
        }
        
        @Override
        protected final void initializeStructFields(HasMetaFields value) {
            value.full_name = this.full_name.initialize();
            value.name = this.name.initialize();
        }
        
        @Override
        protected final void cloneStructFields(HasMetaFields fromValue, HasMetaFields toValue) {
            toValue.full_name = this.full_name.clone(fromValue.full_name);
            toValue.name = this.name.clone(fromValue.name);
        }
    }

    public static final com.microsoft.bond.StructBondType<HasMetaFields> BOND_TYPE = new StructBondTypeImpl.StructBondTypeBuilderImpl().getInitializedFromCache();

    public static void initializeBondType() {
        StructBondTypeImpl.StructBondTypeBuilderImpl.register();
    }

    static {
        initializeBondType();
    }
    

    public java.lang.String full_name;

    public java.lang.String name;
    
    public HasMetaFields() {
        super();
        ((StructBondTypeImpl)BOND_TYPE).initializeStructFields(this);
    };

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof HasMetaFields)) return false;
        
        final HasMetaFields other = (HasMetaFields) o;
        if (!((this.full_name == null && other.full_name == null) || (this.full_name != null && this.full_name.equals(other.full_name)))) return false;
        if (!((this.name == null && other.name == null) || (this.name != null && this.name.equals(other.name)))) return false;
        return true;
    }

    @Override
    public int hashCode() {
        int result = 17;
        result += full_name == null ? 0 : full_name.hashCode();
        result *= 0xeadbeef;
        result ^= result >> 16;
        result += name == null ? 0 : name.hashCode();
        result *= 0xeadbeef;
        result ^= result >> 16;
        return result;
    }

    @Override
    public com.microsoft.bond.StructBondType<? extends HasMetaFields> getBondType() {
        return BOND_TYPE;
    }
}
