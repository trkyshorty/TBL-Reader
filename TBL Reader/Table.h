#pragma once
#include "Windows.h"
#include "ByteBuffer.h"
#include <fstream>
#include <type_traits>
#include <vector>
#include <map>

template<typename Type> struct is_std_vector final : std::false_type {};
template<typename... Type> struct is_std_vector<std::vector<Type...>> final : std::true_type {};

template <typename Type> class Table
{
    public:
        Table() { }
        virtual ~Table() 
        {
            Release();
        }

        void Release()
        {
            m_DataType.clear();
            m_Data.clear();
        }

        bool Load(std::string szFileName)
        {
            std::ifstream ifs(szFileName.c_str(), std::ios::binary | std::ios::ate);
            std::ifstream::pos_type pos = ifs.tellg();

            if (pos == -1)
                return false;

            std::vector<uint8_t> vecBuffer(static_cast<size_t>(pos));

            ifs.seekg(0, std::ios::beg);
            ifs.read(reinterpret_cast<char*>(&vecBuffer[0]), pos);

            std::vector<uint8_t> vecOutputBuffer;
            Decode(vecBuffer, vecOutputBuffer);

            return ReadTable(vecOutputBuffer);
        }

        Type* Find(uint32_t dwID)
        {
            It_Table it = m_Data.find(dwID);
            if (it == m_Data.end()) return NULL;
            else return &(it->second);
        }

        std::map<uint32_t, Type> GetData() { return m_Data; }
        size_t GetDataSize() { return m_Data.size(); }
        size_t GetColumnSize() { return m_DataType.size();}

    protected:
        enum DATA_TYPE { DT_NONE, DT_CHAR, DT_BYTE, DT_SHORT, DT_WORD, DT_INT, DT_DWORD, DT_STRING, DT_FLOAT, DT_DOUBLE, DT_LONG, DT_ULONG };

        typename std::vector<DATA_TYPE> m_DataType;
        typename std::map<uint32_t, Type> m_Data;
        typedef typename std::map<uint32_t, Type>::iterator		It_Table;
        typedef typename std::map<uint32_t, Type>::value_type	Val_Table;
        typedef typename std::pair<typename std::map<uint32_t, Type>::iterator, bool> Pair_Table;

        void Decode(std::vector<uint8_t> vecInputBuffer, std::vector<uint8_t>& vecOutputBuffer)
        {
            int32_t iLength = vecInputBuffer.size() - 20;

            int32_t iInputIndex = 0;
            int32_t iOutputIndex = 0;

            std::vector<uint8_t> vecPlainByteBlock(64);

            vecOutputBuffer.resize(iLength);
            vecOutputBuffer.assign(vecInputBuffer.begin() + 20, vecInputBuffer.end());

            int32_t iMainCounter = (iLength + 7) >> 3;

            do
            {

                vecPlainByteBlock.clear();
                vecPlainByteBlock.resize(64);

                int32_t iIndex = 0;
                int32_t iCounter1 = 8;

                do
                {

                    uint8_t byInput = vecOutputBuffer[iInputIndex++];

                    vecPlainByteBlock[iIndex] = (uint8_t)((byInput >> 7) & 1);

                    int32_t iPlainByteIndex = iIndex + 1;

                    vecPlainByteBlock[iPlainByteIndex++] = (uint8_t)((byInput >> 6) & 1);
                    vecPlainByteBlock[iPlainByteIndex++] = (uint8_t)((byInput >> 5) & 1);
                    vecPlainByteBlock[iPlainByteIndex++] = (uint8_t)((byInput >> 4) & 1);
                    vecPlainByteBlock[iPlainByteIndex++] = (uint8_t)((byInput >> 3) & 1);
                    vecPlainByteBlock[iPlainByteIndex++] = (uint8_t)((byInput >> 2) & 1);
                    vecPlainByteBlock[iPlainByteIndex++] = (uint8_t)((byInput >> 1) & 1);
                    vecPlainByteBlock[iPlainByteIndex] = (uint8_t)(byInput & 1);

                    iIndex = iPlainByteIndex + 1;

                } while (iCounter1-- != 1);

                InitialDecode(vecPlainByteBlock, 0);

                int32_t iCounter2 = 0;

                do
                {

                    uint8_t byProcessedByte = (uint8_t)(vecPlainByteBlock[iCounter2 + 7] | (2 * (vecPlainByteBlock[iCounter2 + 6] | (2 * (vecPlainByteBlock[iCounter2 + 5] | (2 * (vecPlainByteBlock[iCounter2 + 4] | (2 * (vecPlainByteBlock[iCounter2 + 3] | (2 * (vecPlainByteBlock[iCounter2 + 2] | (2 * (vecPlainByteBlock[iCounter2 + 1] | (2 * vecPlainByteBlock[iCounter2]))))))))))))));
                    vecOutputBuffer[iOutputIndex++] = byProcessedByte;
                    iCounter2 += 8;

                } while (iCounter2 < 64);

            } while (iMainCounter-- != 1);

            uint16_t sVolatileKey = 0x0418;
            uint16_t sCipherKey1 = 0x8041;
            uint16_t sCipherKey2 = 0x1804;

            for (size_t i = 0; i < vecOutputBuffer.size(); i++)
            {
                uint8_t byRawByte = vecOutputBuffer[i];
                uint8_t byTemporaryKey = (uint8_t)((sVolatileKey & 0xff00) >> 8);
                uint8_t byEncryptedByte = (uint8_t)(byTemporaryKey ^ byRawByte);
                sVolatileKey = (uint16_t)((byRawByte + sVolatileKey) * sCipherKey1 + sCipherKey2);
                vecOutputBuffer[i] = byEncryptedByte;
            }
        }

        void InitialDecode(std::vector<uint8_t>& vecPlainByteblock, int p2)
        {
            int32_t iStartIndex1 = 0;
            int32_t iStartIndex2 = 15;

            int32_t* iNumArray = new int32_t[8];
            uint8_t* byBuffer = new uint8_t[48];

            int32_t iNumArray1[] = {
                0x10101, 0x100, 0x1000101, 0x1000000, 0x10000, 0x1010101, 0x1010001, 1, 0x1010000, 0x10001, 0x10100, 0x101, 0x1000100, 0x1000001, 0, 0x1010100,
                0, 0x1010101, 0x1010100, 0x100, 0x10101, 0x10000, 0x1000101, 0x1000000, 0x10001, 0x10100, 0x101, 0x1010001, 0x1000001, 0x1000100, 0x1010000, 1,
                0x100, 0x1000000, 0x10101, 1, 0x1000101, 0x10100, 0x10000, 0x1010001, 0x1010101, 0x101, 0x1000001, 0x1010100, 0x1010000, 0x10001, 0x1000100, 0,
                0x1010101, 0x101, 1, 0x10000, 0x100, 0x1000001, 0x1000000, 0x1010100, 0x1000100, 0x1010001, 0x1010000, 0x10101, 0x10001, 0, 0x10100, 0x1000101
            };

            int32_t iNumArray2[] = {
                0x1010101, 0x1000000, 1, 0x10101, 0x10100, 0x1010001, 0x1010000, 0x100, 0x1000001, 0x1010100, 0x10000, 0x1000101, 0x101, 0, 0x1000100, 0x10001,
                0x1010000, 0x1000101, 0x100, 0x1010100, 0x1010101, 0x10000, 1, 0x10101, 0x101, 0, 0x1000000, 0x10001, 0x10100, 0x1000001, 0x1010001, 0x1000100,
                0, 0x10101, 0x1010100, 0x1010001, 0x10001, 0x100, 0x1000101, 0x1000000, 0x1000100, 1, 0x101, 0x10100, 0x1000001, 0x1010000, 0x10000, 0x1010101,
                0x1000101, 1, 0x10001, 0x1000000, 0x1010000, 0x1010101, 0x100, 0x10000, 0x1010001, 0x10100, 0x1010100, 0x101, 0, 0x1000100, 0x10101, 0x1000001
            };

            int32_t iNumArray3[] = {
                0x10001, 0, 0x1000001, 0x10101, 0x10100, 0x1010000, 0x1010101, 0x1000100, 0x1000000, 0x1000101, 0x101, 0x1010100, 0x1010001, 0x100, 0x10000, 1,
                0x1000101, 0x1010100, 0, 0x1000001, 0x1010000, 0x100, 0x10100, 0x10001, 0x10000, 1, 0x1000100, 0x10101, 0x101, 0x1010001, 0x1010101, 0x1000000,
                0x1000101, 0x10100, 0x100, 0x1000001, 1, 0x1010101, 0x1010000, 0, 0x1010001, 0x1000000, 0x10000, 0x101, 0x1000100, 0x10001, 0x10101, 0x1010100,
                0x1000000, 0x10001, 0x1000101, 0, 0x10100, 0x1000001, 1, 0x1010100, 0x100, 0x1010101, 0x10101, 0x1010000, 0x1010001, 0x1000100, 0x10000, 0x101
            };

            int32_t iNumArray4[] = {
                0x1010100, 0x1000101, 0x10101, 0x1010000, 0, 0x10100, 0x1000001, 0x10001, 0x1000000, 0x10000, 1, 0x1000100, 0x1010001, 0x101, 0x100, 0x1010101,
                0x1000101, 1, 0x1010001, 0x1000100, 0x10100, 0x1010101, 0, 0x1010000, 0x100, 0x1010100, 0x10000, 0x101, 0x1000000, 0x10001, 0x10101, 0x1000001,
                0x10001, 0x10100, 0x1000001, 0, 0x101, 0x1010001, 0x1010100, 0x1000101, 0x1010101, 0x1000000, 0x1010000, 0x10101, 0x1000100, 0x10000, 1, 0x100,
                0x1010000, 0x1010101, 0, 0x10100, 0x10001, 0x1000000, 0x1000101, 1, 0x1000001, 0x100, 0x1000100, 0x1010001, 0x101, 0x1010100, 0x10000, 0x10101
            };

            int32_t iNumArray5[] = {
                0x10000, 0x101, 0x100, 0x1000000, 0x1010100, 0x10001, 0x1010001, 0x10100, 1, 0x1000100, 0x1010000, 0x1010101, 0x1000101, 0, 0x10101, 0x1000001,
                0x10101, 0x1010001, 0x10000, 0x101, 0x100, 0x1010100, 0x1000101, 0x1000000, 0x1000100, 0, 0x1010101, 0x10001, 0x1010000, 0x1000001, 1, 0x10100,
                0x100, 0x10000, 0x1000000, 0x1010001, 0x10001, 0x1000101, 0x1010100, 1, 0x1010101, 0x1000001, 0x101, 0x1000100, 0x10100, 0x1010000, 0, 0x10101,
                0x1010001, 1, 0x101, 0x1010100, 0x1000000, 0x10101, 0x10000, 0x1000101, 0x10100, 0x1010101, 0, 0x1000001, 0x10001, 0x100, 0x1000100, 0x1010000
            };

            int32_t iNumArray6[] = {
                0x101, 0x1000000, 0x10001, 0x1010101, 0x1000001, 0x10000, 0x10100, 1, 0, 0x1000101, 0x1010000, 0x100, 0x10101, 0x1010100, 0x1000100, 0x1010001,
                0x10001, 0x1010101, 0x100, 0x10000, 0x1010100, 0x101, 0x1000001, 0x1000100, 0x10100, 0x1000000, 0x1000101, 0x10101, 0, 0x1010001, 0x1010000, 1,
                0x1000001, 0x10101, 0x1010101, 0x1000100, 0x10000, 1, 0x101, 0x1010000, 0x1010100, 0, 0x100, 0x10001, 0x1000000, 0x1000101, 0x1010001, 0x10100,
                0x100, 0x1010000, 0x10000, 0x101, 0x1000001, 0x1000100, 0x1010101, 0x10001, 0x1010001, 0x10101, 0x1000000, 0x1010100, 0x10100, 0, 1, 0x1000101
            };

            int32_t iNumArray7[] = {
                0x100, 0x1010001, 0x10000, 0x10101, 0x1010101, 0, 1, 0x1000101, 0x1010000, 0x101, 0x1000001, 0x1010100, 0x1000100, 0x10001, 0x10100, 0x1000000,
                0x1000101, 0, 0x1010001, 0x1010100, 0x100, 0x1000001, 0x1000000, 0x10001, 0x10101, 0x1010000, 0x1000100, 0x101, 0x10000, 0x1010101, 1, 0x10100,
                0x1000000, 0x100, 0x1010001, 0x1000101, 0x101, 0x1010000, 0x1010100, 0x10101, 0x10001, 0x1010101, 0x10100, 1, 0, 0x1000100, 0x1000001, 0x10000,
                0x10100, 0x1010001, 0x1000101, 1, 0x1000000, 0x100, 0x10001, 0x1010100, 0x1000001, 0x1000100, 0, 0x1010101, 0x10101, 0x10000, 0x1010000, 0x101
            };

            int32_t iNumArray8[] = {
                0x1000101, 0x10000, 1, 0x100, 0x10100, 0x1010101, 0x1010001, 0x1000000, 0x10001, 0x1000001, 0x1010000, 0x10101, 0x1000100, 0, 0x101, 0x1010100,
                0x1000000, 0x1010101, 0x1000101, 1, 0x10001, 0x1010000, 0x1010100, 0x100, 0x101, 0x1000100, 0x10100, 0x1010001, 0, 0x10101, 0x1000001, 0x10000,
                0x1010100, 0x1010001, 0x100, 0x1000000, 0x1000001, 0x101, 0x10101, 0x10000, 0, 0x10100, 0x10001, 0x1000101, 0x1010101, 0x1010000, 0x1000100, 1,
                0x10000, 0x1000000, 0x10101, 0x1010100, 0x100, 0x10001, 1, 0x1000101, 0x1010101, 0x101, 0x1000001, 0, 0x1010000, 0x1000100, 0x10100, 0x1010001
            };

            BYTE byKey[] = {
                0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1,
                1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1,
                1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
                1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
                1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0,
                1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0,
                0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1,
                1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1,
                0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1,
                1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0,
                0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0,
                1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1,
                0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
                0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1,
                1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0,
                1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0,
                1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0,
                1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1,
                0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1,
                0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1,
                1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0,
                1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0,
                1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0,
                0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1,
                0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0,
                1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0,
                0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0,
                0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1,
                1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0,
                1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1,
                1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1,
                0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1,
                1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1,
                1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1,
                1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
                1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
                0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0,
                0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0,
                1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0,
                1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1,
                0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1,
                0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0,
                1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0,
                0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1,
                0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0,
                1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1,
                1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0
            };

            uint8_t byExpansionOperationMatrix[] =
            {
                32, 1,  2,  3,  4,  5,
                4,  5,  6,  7,  8,  9,
                8,  9,  10, 11, 12, 13,
                12, 13, 14, 15, 16, 17,
                16, 17, 18, 19, 20, 21,
                20, 21, 22, 23, 24, 25,
                24, 25, 26, 27, 28, 29,
                28, 29, 30, 31, 32, 1
            };

            BYTE byPermutation[] =
            {
                16, 7, 20, 21, 29, 12, 28, 17,
                1, 15, 23, 26, 5, 18, 31, 10,
                2, 8, 24, 14, 32, 27, 3, 9,
                19, 13, 30, 6, 22, 11, 4, 25
            };

            do
            {

                int32_t iInputIndex = 0;

                do
                {

                    int32_t iStartIndex = iStartIndex1;

                    if (p2 == 0)
                        iStartIndex = iStartIndex2;

                    byBuffer[iInputIndex] = (uint8_t)(byKey[(48 * iStartIndex) + iInputIndex] ^ vecPlainByteblock[byExpansionOperationMatrix[iInputIndex] + 31]);
                    iInputIndex++;

                } while (iInputIndex < 48);

                iNumArray[0] = iNumArray1[byBuffer[4] | (2 * (byBuffer[3] | (2 * (byBuffer[2] | (2 * (byBuffer[1] | (2 * (byBuffer[5] | (2 * byBuffer[0])))))))))];
                iNumArray[1] = iNumArray2[byBuffer[10] | (2 * (byBuffer[9] | (2 * (byBuffer[8] | (2 * (byBuffer[7] | (2 * (byBuffer[11] | (2 * byBuffer[6])))))))))];
                iNumArray[2] = iNumArray3[byBuffer[16] | (2 * (byBuffer[15] | (2 * (byBuffer[14] | (2 * (byBuffer[13] | (2 * (byBuffer[17] | (2 * byBuffer[12])))))))))];
                iNumArray[3] = iNumArray4[byBuffer[22] | (2 * (byBuffer[21] | (2 * (byBuffer[20] | (2 * (byBuffer[19] | (2 * (byBuffer[23] | (2 * byBuffer[18])))))))))];
                iNumArray[4] = iNumArray5[byBuffer[28] | (2 * (byBuffer[27] | (2 * (byBuffer[26] | (2 * (byBuffer[25] | (2 * (byBuffer[29] | (2 * byBuffer[24])))))))))];
                iNumArray[5] = iNumArray6[byBuffer[34] | (2 * (byBuffer[33] | (2 * (byBuffer[32] | (2 * (byBuffer[31] | (2 * (byBuffer[35] | (2 * byBuffer[30])))))))))];
                iNumArray[6] = iNumArray7[byBuffer[40] | (2 * (byBuffer[39] | (2 * (byBuffer[38] | (2 * (byBuffer[37] | (2 * (byBuffer[41] | (2 * byBuffer[36])))))))))];
                iNumArray[7] = iNumArray8[byBuffer[46] | (2 * (byBuffer[45] | (2 * (byBuffer[44] | (2 * (byBuffer[43] | (2 * (byBuffer[47] | (2 * byBuffer[42])))))))))];

                std::vector<uint8_t> destinationArray(32);

                for (size_t i = 0; i < 8; i++)
                {
                    uint8_t* byBytePointer = reinterpret_cast<uint8_t*>(&iNumArray[i]);
                    auto vecNumBuffer = std::vector<uint8_t>(byBytePointer, byBytePointer + sizeof(iNumArray[i]));

                    for (size_t b = 0; b < vecNumBuffer.size(); b++)
                        destinationArray[(i * 4) + b] = vecNumBuffer[b];
                }

                iInputIndex = 0;
                int32_t iCounter = 32;

                if (iStartIndex2 <= 0)
                {

                    do
                    {
                        uint8_t byInput = (uint8_t)(vecPlainByteblock[iInputIndex] ^ destinationArray[byPermutation[iInputIndex] - 1]);

                        vecPlainByteblock[iInputIndex] = byInput;

                        iInputIndex++;

                    } while (iCounter-- > -1);
                }
                else
                {

                    do
                    {

                        uint8_t byInput1 = vecPlainByteblock[iInputIndex + 32];
                        uint8_t byInput2 = (uint8_t)(vecPlainByteblock[iInputIndex] ^ destinationArray[byPermutation[iInputIndex] - 1]);

                        vecPlainByteblock[iInputIndex] = byInput1;
                        vecPlainByteblock[iInputIndex + 32] = byInput2;

                        iInputIndex++;

                    } while (iCounter-- > -1);
                }

                iStartIndex1++;

            } while (iStartIndex2-- > 0);
        }

        bool ReadTable(std::vector<uint8_t> vecBuffer)
        {
            ByteBuffer buffer;
            buffer.append(vecBuffer.data(), vecBuffer.size());

            ////////////////////////////////////////////////////////////
            int32_t iUnknownInteger = buffer.read<int32_t>();
            int8_t byUnknownByte = buffer.read<int8_t>();
            ////////////////////////////////////////////////////////////

            int32_t iColumnCount = buffer.read<int32_t>();

            for (size_t i = 0; i < (size_t)iColumnCount; i++)
                m_DataType.push_back((DATA_TYPE)buffer.read<int32_t>());

            std::vector<int> vecOffsets;
            if (!MakeStructureOffset(vecOffsets))
            {
                m_DataType.clear();
                return false;
            }

            int iSize = vecOffsets[iColumnCount];

            if (sizeof(Type) != iSize ||
                DT_DWORD != m_DataType[0])
            {
                m_DataType.clear();
                return false;
            }

            size_t iRowCount = buffer.read<int32_t>();

            Type Data;

            for (size_t i = 0; i < (size_t)iRowCount; i++)
            {
                for (size_t j = 0; j < (size_t)iColumnCount; j++)
                {
                    switch (m_DataType[j])
                    {
                        case DT_CHAR:
                            *(int8_t*)((char*)(&Data) + vecOffsets[j]) = buffer.read<int8_t>();
                            break;

                        case DT_BYTE:
                            *(uint8_t*)((char*)(&Data) + vecOffsets[j]) = buffer.read<uint8_t>();
                            break;

                        case DT_SHORT:
                            *(int16_t*)((char*)(&Data) + vecOffsets[j]) = buffer.read<int16_t>();
                            break;

                        case DT_WORD:
                            *(uint16_t*)((char*)(&Data) + vecOffsets[j]) = buffer.read<uint16_t>();
                            break;

                        case DT_INT:
                            *(int32_t*)((char*)(&Data) + vecOffsets[j]) = buffer.read<int32_t>();
                            break;

                        case DT_DWORD:
                            *(uint32_t*)((char*)(&Data) + vecOffsets[j]) = buffer.read<uint32_t>();
                            break;

                        case DT_STRING:
                        {
                            int32_t iStringLen = buffer.read<int32_t>();
                            std::string strValue = "";

                            if (iStringLen > 0)
                                buffer.readString(strValue, iStringLen);

                            *(std::string*)((char*)(&Data) + vecOffsets[j]) = strValue;
                        }
                        break;

                        case DT_FLOAT:
                            *(float*)((char*)(&Data) + vecOffsets[j]) = buffer.read<float>();
                            break;

                        case DT_DOUBLE:
                            *(double*)((char*)(&Data) + vecOffsets[j]) = buffer.read<double>();
                            break;

                        case DT_LONG:
                            *(int64_t*)((char*)(&Data) + vecOffsets[j]) = buffer.read<int64_t>();
                            break;

                        case DT_ULONG:
                            *(uint64_t*)((char*)(&Data) + vecOffsets[j]) = buffer.read<uint64_t>();
                            break;

                        case DT_NONE:
                        default:
                            break;
                    }
                }

                uint32_t dwKey = *((uint32_t*)(&Data));
                Pair_Table pt = m_Data.insert(Val_Table(dwKey, Data));
            }

            return true;
        }

        int GetSize(DATA_TYPE DataType) const
        {
            switch (DataType)
            {
                case DT_CHAR:
                    return sizeof(char);
                case DT_BYTE:
                    return sizeof(uint8_t);
                case DT_SHORT:
                    return sizeof(int16_t);
                case DT_WORD:
                    return sizeof(uint16_t);
                case DT_INT:
                    return sizeof(int);
                case DT_DWORD:
                    return sizeof(uint32_t);
                case DT_STRING:
                    return sizeof(std::string);
                case DT_FLOAT:
                    return sizeof(float);
                case DT_DOUBLE:
                    return sizeof(double);
                case DT_LONG:
                    return sizeof(int64_t);
                case DT_ULONG:
                    return sizeof(uint64_t);
            }

            return 0;
        }

        bool MakeStructureOffset(std::vector<int32_t>& vecOffsets)
        {
            if (m_DataType.empty()) return false;

            int32_t iDataTypeCount = m_DataType.size();

            vecOffsets.clear();
            vecOffsets.resize(iDataTypeCount + 1);
            vecOffsets[0] = 0;

            int32_t iPrevDataSize = GetSize(m_DataType[0]);

            for (size_t i = 1; i < (size_t)iDataTypeCount; ++i)
            {
                int32_t iCurDataSize = GetSize(m_DataType[i]);

                if (1 == iCurDataSize % 4)
                {
                    vecOffsets[i] = vecOffsets[i - 1] + iPrevDataSize;
                }
                else if (2 == iCurDataSize % 4)
                {
                    if (0 == ((vecOffsets[i - 1] + iPrevDataSize) % 2))
                        vecOffsets[i] = vecOffsets[i - 1] + iPrevDataSize;
                    else
                        vecOffsets[i] = vecOffsets[i - 1] + iPrevDataSize + 1;
                }
                else if (0 == iCurDataSize % 4)
                {
                    if (0 == ((vecOffsets[i - 1] + iPrevDataSize) % 4))
                        vecOffsets[i] = vecOffsets[i - 1] + iPrevDataSize;
                    else
                        vecOffsets[i] = ((int32_t)(vecOffsets[i - 1] + iPrevDataSize + 3) / 4) * 4;
                }

                iPrevDataSize = iCurDataSize;
            }

            vecOffsets[iDataTypeCount] = ((int32_t)(vecOffsets[iDataTypeCount - 1] + iPrevDataSize + 3) / 4) * 4;

            return true;
        }
};


