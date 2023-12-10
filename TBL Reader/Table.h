#pragma once

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
    Table()
    {
        m_DataType.clear();
        m_Data.clear();
    }

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
        std::ifstream ifs(szFileName, std::ios::binary | std::ios::ate);
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

    void Insert(uint32_t dwID, Type pType)
    {
        It_Table it = m_Data.find(dwID);
        if (it == m_Data.end())
            m_Data.insert(Val_Table(dwID, pType));
    }

    std::map<uint32_t, Type> GetData() { return m_Data; }

    size_t GetDataSize() { return m_Data.size(); }
    size_t GetColumnSize() { return m_DataType.size(); }

protected:
    enum DATA_TYPE { DT_NONE, DT_CHAR, DT_BYTE, DT_SHORT, DT_WORD, DT_INT, DT_DWORD, DT_STRING, DT_FLOAT, DT_DOUBLE, DT_LONG, DT_ULONG };

    typename std::vector<DATA_TYPE> m_DataType;
    typename std::map<uint32_t, Type> m_Data;
    typedef typename std::map<uint32_t, Type>::iterator		It_Table;
    typedef typename std::map<uint32_t, Type>::value_type	Val_Table;
    typedef typename std::pair<typename std::map<uint32_t, Type>::iterator, bool> Pair_Table;

    void Decode(std::vector<uint8_t> vecInputBuffer, std::vector<uint8_t>& vecOutputBuffer)
    {
        int32_t iInputIndex = 0;
        int32_t iOutputIndex = 0;

        std::vector<uint8_t> vecPlainByteBlock(64);

        // Determine the length of the portion of vecInputBuffer to copy
        int32_t iLength = vecInputBuffer.size() - 20;

        // Copy the portion of vecInputBuffer starting from the 20th element to vecOutputBuffer
        vecOutputBuffer.resize(iLength);
        std::copy(vecInputBuffer.begin() + 20, vecInputBuffer.end(), vecOutputBuffer.begin());

        int32_t iMainCounter = (iLength + 7) >> 3;

        for (int32_t i = 0; i < iMainCounter; i++)
        {
            // process input data into 64-byte block
            for (int32_t j = 0; j < 8; j++)
            {
                uint8_t byInput = vecOutputBuffer[iInputIndex++];

                vecPlainByteBlock[j * 8 + 0] = (uint8_t)((byInput >> 7) & 1);
                vecPlainByteBlock[j * 8 + 1] = (uint8_t)((byInput >> 6) & 1);
                vecPlainByteBlock[j * 8 + 2] = (uint8_t)((byInput >> 5) & 1);
                vecPlainByteBlock[j * 8 + 3] = (uint8_t)((byInput >> 4) & 1);
                vecPlainByteBlock[j * 8 + 4] = (uint8_t)((byInput >> 3) & 1);
                vecPlainByteBlock[j * 8 + 5] = (uint8_t)((byInput >> 2) & 1);
                vecPlainByteBlock[j * 8 + 6] = (uint8_t)((byInput >> 1) & 1);
                vecPlainByteBlock[j * 8 + 7] = (uint8_t)(byInput & 1);
            }

            // perform initial decoding
            InitialDecode(vecPlainByteBlock, 0);

            // process output data
            for (int32_t j = 0; j < 8; j++)
            {
                uint8_t byProcessedByte = vecPlainByteBlock[j * 8 + 7] |
                    (vecPlainByteBlock[j * 8 + 6] << 1) |
                    (vecPlainByteBlock[j * 8 + 5] << 2) |
                    (vecPlainByteBlock[j * 8 + 4] << 3) |
                    (vecPlainByteBlock[j * 8 + 3] << 4) |
                    (vecPlainByteBlock[j * 8 + 2] << 5) |
                    (vecPlainByteBlock[j * 8 + 1] << 6) |
                    (vecPlainByteBlock[j * 8 + 0] << 7);

                vecOutputBuffer[iOutputIndex++] = byProcessedByte;
            }
        }

        uint16_t sVolatileKey = 0x0418;
        const uint16_t sCipherKey1 = 0x8041;
        const uint16_t sCipherKey2 = 0x1804;

        for (size_t i = 0, size = vecOutputBuffer.size(); i < size; i++)
        {
            const uint8_t byRawByte = vecOutputBuffer[i];
            const uint8_t byTemporaryKey = static_cast<uint8_t>(sVolatileKey >> 8);
            const uint8_t byEncryptedByte = byTemporaryKey ^ byRawByte;
            sVolatileKey = static_cast<uint16_t>((byRawByte + sVolatileKey) * sCipherKey1 + sCipherKey2);
            vecOutputBuffer[i] = byEncryptedByte;
        }
    }

    void InitialDecode(std::vector<uint8_t>& vecPlainByteblock, int p2)
    {
        int32_t iNumArray1[] =
        {
            0x10101, 0x100, 0x1000101, 0x1000000, 0x10000, 0x1010101, 0x1010001, 1, 0x1010000, 0x10001, 0x10100, 0x101, 0x1000100, 0x1000001, 0, 0x1010100,
            0, 0x1010101, 0x1010100, 0x100, 0x10101, 0x10000, 0x1000101, 0x1000000, 0x10001, 0x10100, 0x101, 0x1010001, 0x1000001, 0x1000100, 0x1010000, 1,
            0x100, 0x1000000, 0x10101, 1, 0x1000101, 0x10100, 0x10000, 0x1010001, 0x1010101, 0x101, 0x1000001, 0x1010100, 0x1010000, 0x10001, 0x1000100, 0,
            0x1010101, 0x101, 1, 0x10000, 0x100, 0x1000001, 0x1000000, 0x1010100, 0x1000100, 0x1010001, 0x1010000, 0x10101, 0x10001, 0, 0x10100, 0x1000101
        };

        int32_t iNumArray2[] =
        {
            0x1010101, 0x1000000, 1, 0x10101, 0x10100, 0x1010001, 0x1010000, 0x100, 0x1000001, 0x1010100, 0x10000, 0x1000101, 0x101, 0, 0x1000100, 0x10001,
            0x1010000, 0x1000101, 0x100, 0x1010100, 0x1010101, 0x10000, 1, 0x10101, 0x101, 0, 0x1000000, 0x10001, 0x10100, 0x1000001, 0x1010001, 0x1000100,
            0, 0x10101, 0x1010100, 0x1010001, 0x10001, 0x100, 0x1000101, 0x1000000, 0x1000100, 1, 0x101, 0x10100, 0x1000001, 0x1010000, 0x10000, 0x1010101,
            0x1000101, 1, 0x10001, 0x1000000, 0x1010000, 0x1010101, 0x100, 0x10000, 0x1010001, 0x10100, 0x1010100, 0x101, 0, 0x1000100, 0x10101, 0x1000001
        };

        int32_t iNumArray3[] =
        {
            0x10001, 0, 0x1000001, 0x10101, 0x10100, 0x1010000, 0x1010101, 0x1000100, 0x1000000, 0x1000101, 0x101, 0x1010100, 0x1010001, 0x100, 0x10000, 1,
            0x1000101, 0x1010100, 0, 0x1000001, 0x1010000, 0x100, 0x10100, 0x10001, 0x10000, 1, 0x1000100, 0x10101, 0x101, 0x1010001, 0x1010101, 0x1000000,
            0x1000101, 0x10100, 0x100, 0x1000001, 1, 0x1010101, 0x1010000, 0, 0x1010001, 0x1000000, 0x10000, 0x101, 0x1000100, 0x10001, 0x10101, 0x1010100,
            0x1000000, 0x10001, 0x1000101, 0, 0x10100, 0x1000001, 1, 0x1010100, 0x100, 0x1010101, 0x10101, 0x1010000, 0x1010001, 0x1000100, 0x10000, 0x101
        };

        int32_t iNumArray4[] =
        {
            0x1010100, 0x1000101, 0x10101, 0x1010000, 0, 0x10100, 0x1000001, 0x10001, 0x1000000, 0x10000, 1, 0x1000100, 0x1010001, 0x101, 0x100, 0x1010101,
            0x1000101, 1, 0x1010001, 0x1000100, 0x10100, 0x1010101, 0, 0x1010000, 0x100, 0x1010100, 0x10000, 0x101, 0x1000000, 0x10001, 0x10101, 0x1000001,
            0x10001, 0x10100, 0x1000001, 0, 0x101, 0x1010001, 0x1010100, 0x1000101, 0x1010101, 0x1000000, 0x1010000, 0x10101, 0x1000100, 0x10000, 1, 0x100,
            0x1010000, 0x1010101, 0, 0x10100, 0x10001, 0x1000000, 0x1000101, 1, 0x1000001, 0x100, 0x1000100, 0x1010001, 0x101, 0x1010100, 0x10000, 0x10101
        };

        int32_t iNumArray5[] =
        {
            0x10000, 0x101, 0x100, 0x1000000, 0x1010100, 0x10001, 0x1010001, 0x10100, 1, 0x1000100, 0x1010000, 0x1010101, 0x1000101, 0, 0x10101, 0x1000001,
            0x10101, 0x1010001, 0x10000, 0x101, 0x100, 0x1010100, 0x1000101, 0x1000000, 0x1000100, 0, 0x1010101, 0x10001, 0x1010000, 0x1000001, 1, 0x10100,
            0x100, 0x10000, 0x1000000, 0x1010001, 0x10001, 0x1000101, 0x1010100, 1, 0x1010101, 0x1000001, 0x101, 0x1000100, 0x10100, 0x1010000, 0, 0x10101,
            0x1010001, 1, 0x101, 0x1010100, 0x1000000, 0x10101, 0x10000, 0x1000101, 0x10100, 0x1010101, 0, 0x1000001, 0x10001, 0x100, 0x1000100, 0x1010000
        };

        int32_t iNumArray6[] =
        {
            0x101, 0x1000000, 0x10001, 0x1010101, 0x1000001, 0x10000, 0x10100, 1, 0, 0x1000101, 0x1010000, 0x100, 0x10101, 0x1010100, 0x1000100, 0x1010001,
            0x10001, 0x1010101, 0x100, 0x10000, 0x1010100, 0x101, 0x1000001, 0x1000100, 0x10100, 0x1000000, 0x1000101, 0x10101, 0, 0x1010001, 0x1010000, 1,
            0x1000001, 0x10101, 0x1010101, 0x1000100, 0x10000, 1, 0x101, 0x1010000, 0x1010100, 0, 0x100, 0x10001, 0x1000000, 0x1000101, 0x1010001, 0x10100,
            0x100, 0x1010000, 0x10000, 0x101, 0x1000001, 0x1000100, 0x1010101, 0x10001, 0x1010001, 0x10101, 0x1000000, 0x1010100, 0x10100, 0, 1, 0x1000101
        };

        int32_t iNumArray7[] =
        {
            0x100, 0x1010001, 0x10000, 0x10101, 0x1010101, 0, 1, 0x1000101, 0x1010000, 0x101, 0x1000001, 0x1010100, 0x1000100, 0x10001, 0x10100, 0x1000000,
            0x1000101, 0, 0x1010001, 0x1010100, 0x100, 0x1000001, 0x1000000, 0x10001, 0x10101, 0x1010000, 0x1000100, 0x101, 0x10000, 0x1010101, 1, 0x10100,
            0x1000000, 0x100, 0x1010001, 0x1000101, 0x101, 0x1010000, 0x1010100, 0x10101, 0x10001, 0x1010101, 0x10100, 1, 0, 0x1000100, 0x1000001, 0x10000,
            0x10100, 0x1010001, 0x1000101, 1, 0x1000000, 0x100, 0x10001, 0x1010100, 0x1000001, 0x1000100, 0, 0x1010101, 0x10101, 0x10000, 0x1010000, 0x101
        };

        int32_t iNumArray8[] =
        {
            0x1000101, 0x10000, 1, 0x100, 0x10100, 0x1010101, 0x1010001, 0x1000000, 0x10001, 0x1000001, 0x1010000, 0x10101, 0x1000100, 0, 0x101, 0x1010100,
            0x1000000, 0x1010101, 0x1000101, 1, 0x10001, 0x1010000, 0x1010100, 0x100, 0x101, 0x1000100, 0x10100, 0x1010001, 0, 0x10101, 0x1000001, 0x10000,
            0x1010100, 0x1010001, 0x100, 0x1000000, 0x1000001, 0x101, 0x10101, 0x10000, 0, 0x10100, 0x10001, 0x1000101, 0x1010101, 0x1010000, 0x1000100, 1,
            0x10000, 0x1000000, 0x10101, 0x1010100, 0x100, 0x10001, 1, 0x1000101, 0x1010101, 0x101, 0x1000001, 0, 0x1010000, 0x1000100, 0x10100, 0x1010001
        };

        BYTE byKey[] =
        {
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

        int32_t iStartIndex1 = 0;
        int32_t iStartIndex2 = 15;

        std::vector<int32_t> iNumArray(8);
        std::vector<uint8_t> byBuffer(48);
        std::vector<uint8_t> destinationArray(32);

        int32_t* iNumArraySet[] = { iNumArray1, iNumArray2, iNumArray3, iNumArray4, iNumArray5, iNumArray6, iNumArray7, iNumArray8 };

        while (iStartIndex2 > -1)
        {
            int32_t iInputIndex = 0;

            while (iInputIndex < 48)
            {
                int32_t iStartIndex = (p2 == 0) ? iStartIndex2 : iStartIndex1;

                byBuffer[iInputIndex] = (uint8_t)(byKey[(48 * iStartIndex) + iInputIndex] ^ vecPlainByteblock[byExpansionOperationMatrix[iInputIndex] + 31]);
                iInputIndex++;
            }

            for (size_t i = 0; i < 8; i++)
            {
                int index = i * 6;
                iNumArray[i] = iNumArraySet[i][byBuffer[index + 4] | (2 * (byBuffer[index + 3] | (2 * (byBuffer[index + 2] | (2 * (byBuffer[index + 1] | (2 * (byBuffer[index + 5] | (2 * byBuffer[index])))))))))];
                uint8_t* byBytePointer = reinterpret_cast<uint8_t*>(&iNumArray[i]);
                std::memcpy(&destinationArray[i * 4], byBytePointer, sizeof(iNumArray[i]));
            }

            iInputIndex = 0;
            int32_t iCounter = 32;

            for (int i = 0; i < iCounter; i++)
            {
                if (iStartIndex2 <= 0)
                {
                    vecPlainByteblock[iInputIndex] ^= destinationArray[byPermutation[iInputIndex] - 1];
                }
                else
                {
                    uint8_t byInput1 = vecPlainByteblock[i + 32];
                    uint8_t byInput2 = vecPlainByteblock[i] ^ destinationArray[byPermutation[i] - 1];

                    vecPlainByteblock[i] = byInput1;
                    vecPlainByteblock[i + 32] = byInput2;
                }

                iInputIndex++;
            }

            iStartIndex2--;
            iStartIndex1++;
        }
    }

    bool ReadTable(std::vector<uint8_t>& vecBuffer)
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

        //int iSize = vecOffsets[iColumnCount];
        //if (sizeof(Type) != iSize ||
        //    DT_DWORD != m_DataType[0])
        //{
        //    m_DataType.clear();
        //    return false;
        //}

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

            switch (iCurDataSize % 4)
            {
            case 1:
                vecOffsets[i] = vecOffsets[i - 1] + iPrevDataSize;
                break;
            case 2:
                vecOffsets[i] = ((vecOffsets[i - 1] + iPrevDataSize) % 2 == 0) ?
                    vecOffsets[i - 1] + iPrevDataSize :
                    vecOffsets[i - 1] + iPrevDataSize + 1;
                break;
            case 0:
                vecOffsets[i] = ((vecOffsets[i - 1] + iPrevDataSize) % 4 == 0) ?
                    vecOffsets[i - 1] + iPrevDataSize :
                    ((vecOffsets[i - 1] + iPrevDataSize + 3) / 4) * 4;
                break;
            }

            iPrevDataSize = iCurDataSize;
        }

        vecOffsets[iDataTypeCount] = ((vecOffsets[iDataTypeCount - 1] + iPrevDataSize + 3) / 4) * 4;

        return true;
    }
};