{-# LANGUAGE FlexibleContexts, ScopedTypeVariables, MultiWayIf, MultiParamTypeClasses, FlexibleInstances, EmptyDataDecls #-}
module Bond.CompactBinary (
    runCompactBinaryV2Get,
    runCompactBinaryV2Put,
    encodeInt,      -- export for testing
    decodeInt       -- export for testing
  ) where

import Bond.BinaryProto
import Bond.Types
import Bond.Wire
import Control.Applicative
import Data.Binary.Get
import Data.Binary.Put
import Data.Bits
import qualified Data.ByteString.Lazy as Lazy

data CompactBinaryV2Proto

newtype EncodedWord = EncodedWord { unWord :: Word64 }
newtype EncodedInt = EncodedInt { unInt :: Int64 }
newtype Unpacked16 = Unpacked16 { unpack16 :: Word16 }

encodeInt :: EncodedInt -> EncodedWord
encodeInt (EncodedInt i) | i >= 0 = EncodedWord $ 2 * fromIntegral i
encodeInt (EncodedInt i) = EncodedWord $ (2 * fromIntegral (abs i)) - 1

decodeInt :: EncodedWord -> EncodedInt
decodeInt (EncodedWord w) | even w = EncodedInt $ fromIntegral (w `div` 2)
decodeInt (EncodedWord w) = EncodedInt $ negate $ fromIntegral (w `div` 2)

instance BondBinary CompactBinaryV2Proto EncodedWord where
    bondGet = EncodedWord <$> step 0
        where
        step :: Int -> BondGet t Word64
        step n | n > 9 = fail "EncodedWord: sequence too long"
        step n = do
            b <- fromIntegral <$> BondGet getWord8
            rest <- if b `testBit` 7 then step (n + 1)  else return (0 :: Word64)
            return $ (b `clearBit` 7) .|. (rest `shiftL` 7)
    bondPut (EncodedWord i) | i < 128 = BondPut $ putWord8 $ fromIntegral i
    bondPut (EncodedWord i) = do
        let iLow = fromIntegral $ i .&. 0x7F
        BondPut $ putWord8 $ iLow `setBit` 7
        bondPut $ EncodedWord (i `shiftR` 7)

instance BondBinary CompactBinaryV2Proto EncodedInt where
    bondGet = decodeInt <$> bondGet
    bondPut = bondPut . encodeInt

instance BondBinary CompactBinaryV2Proto Unpacked16 where
    bondGet = Unpacked16 <$> BondGet getWord16le
    bondPut = BondPut . putWord16le . unpack16

instance BondBinary CompactBinaryV2Proto Word16 where
    bondGet = fromIntegral . unWord <$> bondGet
    bondPut = bondPut . EncodedWord . fromIntegral

instance BondBinary CompactBinaryV2Proto Word32 where
    bondGet = fromIntegral . unWord <$> bondGet
    bondPut = bondPut . EncodedWord . fromIntegral

instance BondBinary CompactBinaryV2Proto Word64 where
    bondGet = fromIntegral . unWord <$> bondGet
    bondPut = bondPut . EncodedWord . fromIntegral

instance BondBinary CompactBinaryV2Proto Int16 where
    bondGet = fromIntegral . unInt . decodeInt <$> bondGet
    bondPut = bondPut . encodeInt . EncodedInt . fromIntegral

instance BondBinary CompactBinaryV2Proto Int32 where
    bondGet = fromIntegral . unInt . decodeInt <$> bondGet
    bondPut = bondPut . encodeInt . EncodedInt . fromIntegral

instance BondBinary CompactBinaryV2Proto Int64 where
    bondGet = fromIntegral . unInt . decodeInt <$> bondGet
    bondPut = bondPut . encodeInt . EncodedInt . fromIntegral

instance BondBinary CompactBinaryV2Proto FieldTag where
    bondPut (FieldTag t (Ordinal o)) | o <= 5 = let tag = fromWireType t
                                                    ordn = fromIntegral o
                                                    v = (ordn `shiftL` 5) .|. tag
                                                in bondPut v
    bondPut (FieldTag t (Ordinal o)) | o <= 0xff = let tag = fromWireType t .|. 0xc0
                                                       ordn = fromIntegral o :: Word8
                                                    in do
                                                        bondPut tag
                                                        bondPut ordn
    bondPut (FieldTag t (Ordinal o)) = let tag = fromWireType t .|. 0xe0
                                        in do
                                            bondPut tag
                                            bondPut $ Unpacked16 o
    bondGet = do
        tag <- bondGet
        let t = toWireType (tag .&. 0x1f)
        let hibits = tag `shiftR` 5
        o <- if t == BT_STOP || t == BT_STOP_BASE
                then return 0
                else if | hibits <= 5 -> return $ fromIntegral hibits
                        | hibits == 6 -> fromIntegral <$> (bondGet :: BondGet CompactBinaryV2Proto Word8)
                        | otherwise -> unpack16 <$> bondGet
        return $ FieldTag t (Ordinal o)

instance BondBinary CompactBinaryV2Proto ListHead where
    bondPut (ListHead t n) | n < 7 = let tag = fromWireType t
                                         v = tag .|. fromIntegral ((n + 1) `shiftL` 5)
                                      in bondPut v
    bondPut (ListHead t n) = do
        bondPut t
        bondPut $ VarInt n
    bondGet = do
        tag <- bondGet
        let t = toWireType (tag .&. 0x1f)
        let hibits = tag `shiftR` 5
        n <- if hibits == 0
                then fromVarInt <$> bondGet
                else return $ fromIntegral (hibits - 1)
        return $ ListHead t n

instance BondBinaryProto CompactBinaryV2Proto where
    readStruct name reader = do
        VarInt n <- bondGet
        b <- BondGet bytesRead
        r <- reader
        b' <- BondGet bytesRead
        return r
    putStruct name (BondPut writer) = do
        let bs = runPut writer
        bondPut $ VarInt $ fromIntegral $ Lazy.length bs
        BondPut $ putLazyByteString bs
    skipValue = skipCompactV2Value

skipCompactV2Value :: BondBinary CompactBinaryV2Proto FieldTag => ItemType -> BondGet CompactBinaryV2Proto ()
skipCompactV2Value BT_STRUCT = do
    VarInt len <- bondGet
    BondGet $ skip len
skipCompactV2Value tag = skipValue' tag

runCompactBinaryV2Get :: BondGet CompactBinaryV2Proto a -> Lazy.ByteString -> Either (Lazy.ByteString, Int64, String) (Lazy.ByteString, Int64, a) 
runCompactBinaryV2Get (BondGet g) = runGetOrFail g

runCompactBinaryV2Put :: BondPut CompactBinaryV2Proto -> Lazy.ByteString
runCompactBinaryV2Put (BondPut p) = runPut p
