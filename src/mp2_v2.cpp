#include <stdio.h>
#include <cstdint>
#include <stdint.h>
#include <string>
#include <cinttypes>
#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdarg>

#define NOT_VALID  -1

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86))
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

//=============================================================================================================================================================================
// Byte swap
//=============================================================================================================================================================================
#if defined(_MSC_VER)
static inline uint16_t xSwapBytes16(uint16_t Value) { return _byteswap_ushort(Value); }
static inline  int16_t xSwapBytes16(int16_t Value) { return _byteswap_ushort(Value); }
static inline uint32_t xSwapBytes32(uint32_t Value) { return _byteswap_ulong(Value); }
static inline  int32_t xSwapBytes32(int32_t Value) { return _byteswap_ulong(Value); }
static inline uint64_t xSwapBytes64(uint64_t Value) { return _byteswap_uint64(Value); }
static inline  int64_t xSwapBytes64(int64_t Value) { return _byteswap_uint64(Value); }
#elif defined (__GNUC__)
static inline uint16_t xSwapBytes16(uint16_t Value) { return __builtin_bswap16(Value); }
static inline  int16_t xSwapBytes16(int16_t Value) { return __builtin_bswap16(Value); }
static inline uint32_t xSwapBytes32(uint32_t Value) { return __builtin_bswap32(Value); }
static inline  int32_t xSwapBytes32(int32_t Value) { return __builtin_bswap32(Value); }
static inline uint64_t xSwapBytes64(uint64_t Value) { return __builtin_bswap64(Value); }
static inline  int64_t xSwapBytes64(int64_t Value) { return __builtin_bswap64(Value); }
#else
#error Unrecognized compiler
#endif




uint64_t convertFrom8To64(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,
    uint8_t b5, uint8_t b6, uint8_t b7, uint8_t b8)

{
    uint64_t result = 0x0000;

    result = b1;
    result = result << 8;
    result |= b2;
    result = result << 8;
    result |= b3;
    result = result << 8;
    result |= b4;
    result = result << 8;
    result |= b5;
    result = result << 8;
    result |= b6;
    result = result << 8;
    result |= b7;
    result = result << 8;
    result |= b8;

    return result;
}

uint32_t convertFrom8To32(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth) {
    uint32_t result = 0x0000;

    result = first;
    result = result << 8;
    result |= second;
    result = result << 8;
    result |= third;
    result = result << 8;
    result |= fourth;

    return result;
}
uint32_t convertFrom8To24(uint8_t first, uint8_t second, uint8_t third) {
    uint32_t result = 0x0000;

    result = first;
    result = result << 8;
    result |= second;
    result = result << 8;
    result |= third;

    return result;
}


uint16_t convertFrom8To16(uint8_t hi, uint8_t lo) {
    uint16_t result = 0x0000;

    result = hi;
    result = result << 8;
    result |= lo;
    return result;
}



template<class T>
T connectBinaryWords(uint8_t num ...) {
    T result = 0x0000;

    va_list ap;
    va_start(ap, num);
    result = va_arg(ap, uint8_t);

    for (int i = 1; i < num; i++) {
        result = result << 8;
        result |= va_arg(ap, uint8_t);

    }

    return result;
}


/*
MPEG-TS packet:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |                             Header                            | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   4 |                  Adaptation field + Payload                   | `
`     |                                                               | `
` 184 |                                                               | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `


MPEG-TS packet header:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |       SB      |E|S|T|           PID           |TSC|AFC|   CC  | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `

Sync byte                    (SB ) :  8 bits
Transport error indicator    (E  ) :  1 bit
Payload unit start indicator (S  ) :  1 bit
Transport priority           (T  ) :  1 bit
Packet Identifier            (PID) : 13 bits
Transport scrambling control (TSC) :  2 bits
Adaptation field control     (AFC) :  2 bits
Continuity counter           (CC ) :  4 bits
*/


//=============================================================================================================================================================================

class xTS
{
public:
    static constexpr uint32_t TS_PacketLength = 188;
    static constexpr uint32_t TS_HeaderLength = 4;

    static constexpr uint32_t PES_HeaderLength = 6;

    static constexpr uint32_t BaseClockFrequency_Hz = 90000; //Hz
    static constexpr uint32_t ExtendedClockFrequency_Hz = 27000000; //Hz
    static constexpr uint32_t BaseClockFrequency_kHz = 90; //kHz
    static constexpr uint32_t ExtendedClockFrequency_kHz = 27000; //kHz
    static constexpr uint32_t BaseToExtendedClockMultiplier = 300;
};

//=============================================================================================================================================================================

class xTS_PacketHeader
{
public:
    enum class ePID : uint16_t
    {
        PAT = 0x0000,
        CAT = 0x0001,
        TSDT = 0x0002,
        IPMT = 0x0003,
        NIT = 0x0010, //DVB specific PID
        SDT = 0x0011, //DVB specific PID
        NuLL = 0x1FFF,
    };

protected:
    //TODO - header fields
    uint32_t header;
    uint8_t SB;
    uint8_t E;
    uint8_t S;
    uint8_t T;
    uint16_t PID;
    uint8_t TSC;
    uint8_t AFC;
    uint8_t CC;

public:
    void     Reset();
    int32_t  Parse(const uint8_t* Input);
    void     Print() const;

public:

    uint8_t getSyncByte();
    uint8_t getTransportErrorIndicator();
    uint8_t getPayloadUnitStartIndicator();
    uint8_t getTransportPriority();
    uint16_t getPID();
    uint8_t getTransportScramblingControl();
    uint8_t getAdaptationFieldControl();
    uint8_t getContinuityCounter();
public:

    bool     hasAdaptationField() const;
};

//=============================================================================================================================================================================

class xTS_AdaptationField
{
protected:
    //AF length
    uint8_t AFL;
    //mandatory fields
    uint8_t AFC;
    uint8_t FLAGS;
    uint8_t Stuffing;
    bool DC;
    bool RA;
    bool SP;
    bool PR;
    bool OR;
    bool SPF;
    bool TP;
    bool EX;
    //optional fields
    uint64_t PCR_base;
    uint8_t PR_reserved;
    uint16_t PCR_extension;

    uint64_t OPCR_base;
    uint8_t OR_reserved;
    uint16_t OPCR_extension;

public:
    void Reset();
    int32_t Parse(const uint8_t* Input, uint8_t AdaptationFieldControl);
    void Print() const;
public:
    //derrived values
    uint32_t getNumBytes() const;
    uint8_t getAdaptationFieldLenght();

};

//=============================================================================================================================================================================


class xPES_PacketHeader
{
public:
    enum eStreamId : uint8_t
    {
        eStreamId_program_stream_map = 0xBC,
        eStreamId_padding_stream = 0xBE,
        eStreamId_private_stream_2 = 0xBF,
        eStreamId_ECM = 0xF0,
        eStreamId_EMM = 0xF1,
        eStreamId_program_stream_directory = 0xFF,
        eStreamId_DSMCC_stream = 0xF2,
        eStreamId_ITUT_H222_1_type_E = 0xF8,
    };
protected:
    //PES packet header
    uint32_t m_PacketStartCodePrefix;
    uint8_t m_StreamId;
    uint16_t m_PacketLength;
    uint16_t m_HeaderLength;
    uint8_t PTS_DTS_flags;
    uint64_t PTS;
    uint64_t DTS;
public:
    void Reset();
    int32_t Parse(const uint8_t* Input);
    void Print() const;
public:
    //PES packet header
    uint32_t getPacketStartCodePrefix() const;
    uint8_t getStreamId() const;
    uint16_t getPacketLength() const;
    uint8_t getHeaderLength() const;
};

//=============================================================================================================================================================================


class xPES_Assembler
{
public:
    enum class eResult : int32_t
    {
        UnexpectedPID = 1,
        StreamPackedLost,
        AssemblingStarted,
        AssemblingContinue,
        AssemblingFinished,
    };
protected:
    //setup
    int32_t m_PID;
    //buffer
    uint8_t* m_Buffer;
    uint32_t m_BufferSize;
    //operation
    int8_t m_LastContinuityCounter;
    bool m_Started;
    xPES_PacketHeader m_PESH;
public:
    xPES_Assembler() {}
    ~xPES_Assembler() { delete m_Buffer; }
    void Init(int32_t PID);
    eResult AbsorbPacket(const uint8_t* TransportStreamPacket, xTS_PacketHeader* PacketHeader, xTS_AdaptationField* AdaptationField);
    void PrintPESH() const;
    uint8_t getHeaderLenght() const;
    uint8_t* getPacket();
    int32_t getNumPacketBytes() const;
    void xBufferReset();
protected:

    void xBufferAppend(const uint8_t* Data, uint32_t Size);


};

//=============================================================================================================================================================================




int main(int argc, char* argv[], char* envp[])
{


    FILE* TransportStreamFile = fopen("example_new.ts", "rb");

    if (TransportStreamFile == NULL) {
        printf("wrong file name\n");
        return EXIT_FAILURE;
    }

    FILE* AudioMP2 = fopen("PID136.mp2", "wb");
    FILE* Video264 = fopen("PID174.264", "wb");

    uint8_t TS_PacketBuffer[xTS::TS_PacketLength];
    xTS_PacketHeader TS_PacketHeader;
    xTS_AdaptationField TS_PacketAdaptationField;

    xPES_Assembler PES_Assembler136;
    xPES_Assembler PES_Assembler174;
    PES_Assembler136.Init(136);
    PES_Assembler174.Init(174);

    int32_t TS_PacketId = 0;

    while (!feof(TransportStreamFile))
    {
        size_t NumRead = fread(TS_PacketBuffer, 1, xTS::TS_PacketLength, TransportStreamFile);


        if (NumRead != xTS::TS_PacketLength) { break; }

        TS_PacketHeader.Reset();
        TS_PacketHeader.Parse(TS_PacketBuffer);
        TS_PacketAdaptationField.Reset();
        uint8_t temp = TS_PacketHeader.getSyncByte();

        if (TS_PacketHeader.getSyncByte() == 'G' && (TS_PacketHeader.getPID() == 136 || TS_PacketHeader.getPID() == 174))//  && TS_PacketId<50000)
        {
            if (TS_PacketHeader.hasAdaptationField())
            {
                TS_PacketAdaptationField.Parse(TS_PacketBuffer, TS_PacketHeader.getAdaptationFieldControl());
            }

            printf("%010d ", TS_PacketId);
            TS_PacketHeader.Print();

            if (TS_PacketHeader.hasAdaptationField())
            {
                TS_PacketAdaptationField.Print();
            }

            if (TS_PacketHeader.getPID() == 174) {


                xPES_Assembler::eResult Result = PES_Assembler174.AbsorbPacket(TS_PacketBuffer, &TS_PacketHeader, &TS_PacketAdaptationField);
                switch (Result)
                {
                case xPES_Assembler::eResult::StreamPackedLost: printf("PcktLost "); break;
                case xPES_Assembler::eResult::AssemblingStarted:
                    printf("Started ");
                    PES_Assembler174.PrintPESH();
                    break;
                case xPES_Assembler::eResult::AssemblingContinue:
                    printf("Continue ");
                    break;

                case xPES_Assembler::eResult::AssemblingFinished: { printf("Finished ");

                    printf("PES: Len=%d", PES_Assembler174.getNumPacketBytes());

                    fwrite(PES_Assembler174.getPacket(), 1, PES_Assembler174.getNumPacketBytes(), Video264);

                    PES_Assembler174.xBufferReset();
                }break;
                default: break;
                }

                printf("\n");

            }
            else if (TS_PacketHeader.getPID() == 136) {

                xPES_Assembler::eResult Result = PES_Assembler136.AbsorbPacket(TS_PacketBuffer, &TS_PacketHeader, &TS_PacketAdaptationField);
                switch (Result)
                {
                case xPES_Assembler::eResult::StreamPackedLost: printf("PcktLost "); break;
                case xPES_Assembler::eResult::AssemblingStarted:
                    printf("Started ");
                    PES_Assembler136.PrintPESH();
                    break;
                case xPES_Assembler::eResult::AssemblingContinue:
                    printf("Continue ");
                    break;

                case xPES_Assembler::eResult::AssemblingFinished: { printf("Finished ");

                    printf("PES: Len=%d", PES_Assembler136.getNumPacketBytes());

                    fwrite(PES_Assembler136.getPacket(), 1, PES_Assembler136.getNumPacketBytes(), AudioMP2);

                    PES_Assembler136.xBufferReset();
                }break;
                default: break;
                }

                printf("\n");

            }

        }

        TS_PacketId++;

    }

    fclose(TransportStreamFile);
    fclose(AudioMP2);
    fclose(Video264);

}


//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================


int32_t xTS_PacketHeader::Parse(const uint8_t* Input) {

    xTS_PacketHeader::SB = Input[0];
    xTS_PacketHeader::E = (Input[1] & 0x80) ? 1 : 0;
    xTS_PacketHeader::S = (Input[1] & 0x40) ? 1 : 0;
    xTS_PacketHeader::T = (Input[1] & 0x20) ? 1 : 0;
    xTS_PacketHeader::PID = convertFrom8To16((Input[1] & 0x1F), Input[2]);
    xTS_PacketHeader::TSC = (Input[3] & 0xC0) >> 6;
    xTS_PacketHeader::AFC = (Input[3] & 0x30) >> 4;
    xTS_PacketHeader::CC = Input[3] & 0xF;
    header = convertFrom8To32(Input[0], Input[1], Input[2], Input[3]);
    return header;
}

void xTS_PacketHeader::Reset() {
    SB, E, S, T, PID, TSC, AFC, CC = 0;

}

void xTS_PacketHeader::Print() const {
    printf("TS : ");
    printf("SB=%d ", SB);
    printf("E=%d ", E);
    printf("S=%d ", S);
    printf("T=%d ", T);
    printf("PID=%d ", PID);
    printf("TSC=%d ", TSC);
    printf("AFC=%d ", AFC);
    printf("CC=%d ", CC);

}


bool xTS_PacketHeader::hasAdaptationField() const {
    if ((AFC == 2) || (AFC == 3))
        return true;
    else
        return false;
}

uint8_t xTS_PacketHeader::getSyncByte() {
    return SB;
}
uint8_t xTS_PacketHeader::getTransportErrorIndicator() {
    return (header & 0x800000) ? 1 : 0;
}
uint8_t xTS_PacketHeader::getPayloadUnitStartIndicator() {
    return S;
}
uint8_t xTS_PacketHeader::getTransportPriority() {
    return T;
}
uint16_t xTS_PacketHeader::getPID() {
    return PID;
}
uint8_t xTS_PacketHeader::getTransportScramblingControl() {
    return TSC;
}
uint8_t xTS_PacketHeader::getAdaptationFieldControl() {
    return AFC;
}
uint8_t xTS_PacketHeader::getContinuityCounter() {
    return CC;
}



//=============================================================================================================================================================================
// xTS_AdaptationField
//=============================================================================================================================================================================


int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdaptationFieldControl) {
    int pointer = 4;
    AFC = AdaptationFieldControl;
    if ((AdaptationFieldControl == 1) || (AdaptationFieldControl == 3)) {
        AFL = Input[pointer++]; //4
        FLAGS = Input[pointer++];  //5
        Stuffing = AFL - 1;
    }

    else {
        AFL = 0;
        FLAGS = 0;
        Stuffing = 0;
    }

    DC = (FLAGS & 0x80) >> 7;
    RA = (FLAGS & 0x40) >> 6;
    SP = (FLAGS & 0x20) >> 5;
    PR = (FLAGS & 0x10) >> 4;
    OR = (FLAGS & 0x08) >> 3;
    SPF = (FLAGS & 0x04) >> 2;
    TP = (FLAGS & 0x02) >> 1;
    EX = FLAGS & 0x01;


    //Is PCR_FLAG
    if (PR) {

        PCR_base = convertFrom8To64(0, 0, 0,
            Input[pointer], Input[pointer + 1], Input[pointer + 2], Input[pointer + 3], Input[pointer + 4]);
        pointer += 4;

        PCR_base >>= 7;

        PR_reserved = (Input[pointer] & 0x7E);
        PR_reserved >>= 1;
        PCR_extension = Input[pointer] & 0x1;
        PCR_extension = convertFrom8To16(PCR_extension, Input[pointer + 1]);
        pointer += 1;
        Stuffing -= 6;
    }
    //Is OPCR_FLAG
    if (OR) {
        OPCR_base = convertFrom8To64(0, 0, 0,
            Input[pointer], Input[pointer + 1], Input[pointer + 2], Input[pointer + 3], Input[pointer + 4]);
        pointer += 4;

        PCR_base >>= 7;

        OR_reserved = (Input[pointer] & 0x7E);
        OR_reserved >>= 1;
        OPCR_extension = Input[pointer] & 0x1;
        OPCR_extension = convertFrom8To16(PCR_extension, Input[pointer + 1]);
        pointer += 1;
        Stuffing -= 6;
    }
    if (SPF) {
        Stuffing -= 1;
    }
    if (TP) {
        Stuffing -= 1;
    }
    if (EX) {
        Stuffing -= 2;
    }

    return 0;
}
void xTS_AdaptationField::Reset() {
    AFL = 0;
    FLAGS = 0;
    PCR_base = 0;
    PR_reserved = 0;
    PCR_extension = 0;
    OPCR_base = 0;
    OR_reserved = 0;
    OPCR_extension = 0;
    DC, RA, SP, PR, OR, SPF, TP, EX = 0;
}

void xTS_AdaptationField::Print() const {

    if (AFL != 0) {
        printf("AF : ");
        printf("AFL=%d ", AFL);
        printf("DC=%d ", (DC) ? 1 : 0);
        printf("RA=%d ", (RA) ? 1 : 0);
        printf("SP=%d ", (SP) ? 1 : 0);
        printf("PR=%d ", (PR) ? 1 : 0);
        printf("OR=%d ", (OR) ? 1 : 0);
        printf("SP=%d ", (SP) ? 1 : 0);
        printf("TP=%d ", (TP) ? 1 : 0);
        printf("EX=%d ", (EX) ? 1 : 0);
    }
    if (PR) { printf("PCR=%d ", (PCR_base * xTS::BaseToExtendedClockMultiplier) + PCR_extension); }
    if (OR) { printf("OPCR=%d ", (OPCR_base * xTS::BaseToExtendedClockMultiplier) + OPCR_extension); }

    if (AFC == 3) {
        printf("Stuffing: %d ", Stuffing);
    }


}
uint32_t xTS_AdaptationField::getNumBytes() const {
    if (AFL == 0)
        return 0;
    else
        return AFL + 1;
}
uint8_t xTS_AdaptationField::getAdaptationFieldLenght() {
    return AFL;
}


//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================


int32_t xPES_PacketHeader::Parse(const uint8_t* Input) {
    m_PacketStartCodePrefix = convertFrom8To24(Input[0], Input[1], Input[2]);
    m_StreamId = Input[3];
    m_PacketLength = convertFrom8To16(Input[4], Input[5]);
    if (m_PacketLength == 0)
        // printf("PES packet length is neither specified nor bounded and is allowed only in PES packets whose payload consists of bytes from a video elementary stream contained in transport stream packets.");
        m_HeaderLength = 6;


    if (
        m_StreamId != 0xBC &&
        m_StreamId != 0xBE &&
        m_StreamId != 0xBF &&
        m_StreamId != 0xF0 &&
        m_StreamId != 0xF1 &&
        m_StreamId != 0xFF &&
        m_StreamId != 0xF2 &&
        m_StreamId != 0xF8) {
        m_HeaderLength = 9;


        PTS_DTS_flags = (Input[7] & 0b11000000) >> 6;

        if ((Input[7] & 0b11000000) == 0b11000000) {

            m_HeaderLength += 10;

            PTS = Input[9] & 0b1110;
            PTS <<= 7;
            PTS += Input[10];
            PTS <<= 8;
            PTS += Input[11] & 0b11111110;
            PTS <<= 7;
            PTS += Input[12];
            PTS <<= 7;
            PTS += Input[13] >> 1;


            DTS = Input[14] & 0b1110;
            DTS <<= 7;
            DTS += Input[15];
            DTS <<= 8;
            DTS += Input[16] & 0b11111110;
            DTS <<= 7;
            DTS += Input[17];
            DTS <<= 7;
            DTS += Input[18] >> 1;


        }
        else if ((Input[7] & 0b10000000) == 0b10000000) {
            m_HeaderLength += 5;

            PTS = Input[9] & 0b1110;
            PTS <<= 7;
            PTS += Input[10];
            PTS <<= 8;
            PTS += Input[11] & 0b11111110;
            PTS <<= 7;
            PTS += Input[12];
            PTS <<= 7;
            PTS += Input[13] >> 1;

        }

        //ESCR_flag == '1'
        if (Input[7] & 0b00100000) {
            m_HeaderLength += 6;
        }
        //ES_rate_flag == '1'
        if (Input[7] & 0b00010000) {
            m_HeaderLength += 3;
        }
        //DSM_trick_mode_flag == '1'
        if (Input[7] & 0b00001000) {
            m_HeaderLength += 0;
        }
        //additional_copy_info_flag == '1'
        if (Input[7] & 0b00000100) {
            m_HeaderLength += 1;
        }
        //PES_CRC_flag == '1')
        if (Input[7] & 0b00000010) {
            m_HeaderLength += 2;
        }
        //PES_extension_flag == '1'
        if (Input[7] & 0b00000001) {
            int point = m_HeaderLength;
            m_HeaderLength += 1;
            //PES_private_data_flag == '1' -> 128
            if (Input[point] & 0b10000000) {
                m_HeaderLength += 16;
            }
            //pack_header_field_flag == '1' -> 8
            if (Input[point] & 0b01000000) {
                m_HeaderLength += 1;
            }
            //program packet sequence counter flag
            if (Input[point] & 0b00100000) {
                m_HeaderLength += 2;
            }
            //P-STD buffer flag
            if (Input[point] & 0b00010000) {
                m_HeaderLength += 2;
            }
            //P-STD buffer flag
            if (Input[point] & 0b00000001) {
                m_HeaderLength += 2;
            }

        }

    }

    return m_PacketStartCodePrefix;
}

void xPES_PacketHeader::Print() const {
    printf("PES : ");
    printf("PSCP=%d ", m_PacketStartCodePrefix);
    printf("SID=%d ", m_StreamId);
    printf("PacketLength=%d ", m_PacketLength);
    if (PTS_DTS_flags == 0b10)
        printf("PTS=%d ", PTS);

    if (PTS_DTS_flags == 0b11) {
        printf("PTS=%d ", PTS);
        printf("DTS=%d ", DTS);
    }
};

void xPES_PacketHeader::Reset() {
    m_PacketStartCodePrefix, m_StreamId, m_PacketLength, m_HeaderLength, PTS_DTS_flags = 0;
    PTS, DTS = 0;
};



uint8_t xPES_PacketHeader::getHeaderLength() const { return m_HeaderLength; }
uint16_t xPES_PacketHeader::getPacketLength() const { return m_PacketLength; }
uint32_t xPES_PacketHeader::getPacketStartCodePrefix() const { return m_PacketStartCodePrefix; }
uint8_t xPES_PacketHeader::getStreamId() const { return m_StreamId; }

//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================

void xPES_Assembler::Init(int32_t PID) {
    m_PID = PID;
    m_Buffer = new uint8_t[100000];
    m_BufferSize = 0;
};

void xPES_Assembler::PrintPESH() const { m_PESH.Print(); }

uint8_t xPES_Assembler::getHeaderLenght() const { return m_PESH.getHeaderLength(); }

uint8_t* xPES_Assembler::getPacket() { return m_Buffer; }

int32_t xPES_Assembler::getNumPacketBytes() const { return m_BufferSize; }


//!!!!!!!!!!!!!!!!!!!!!!
xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(
    const uint8_t* TransportStreamPacket,
    xTS_PacketHeader* PacketHeader,
    xTS_AdaptationField* AdaptationField) {



    uint8_t TS_AdaptationLenght = 0;
    if (PacketHeader->hasAdaptationField()) {
        TS_AdaptationLenght = AdaptationField->getNumBytes();
    }

    uint32_t tempSize = xTS::TS_PacketLength - xTS::TS_HeaderLength - TS_AdaptationLenght;


    if (PacketHeader->getPayloadUnitStartIndicator()) {

        tempSize = xTS::TS_PacketLength - xTS::TS_HeaderLength - TS_AdaptationLenght;
        uint32_t sizeToSkip = xTS::TS_PacketLength - tempSize;
        uint8_t* firstPacketBuffer = new uint8_t[188];

        for (int i = 0; i < tempSize; i++) {
            firstPacketBuffer[i] = TransportStreamPacket[sizeToSkip + i];

        }
        m_PESH.Parse(firstPacketBuffer);

        tempSize -= getHeaderLenght();

        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
        return eResult::AssemblingStarted;
    }
    else if (PacketHeader->hasAdaptationField()) {


        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);

        return eResult::AssemblingFinished;
    }
    else {

        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);

        return eResult::AssemblingContinue;
    }

}
void xPES_Assembler::xBufferReset() {

    m_BufferSize = 0;

}
void xPES_Assembler::xBufferAppend(const uint8_t* Data, uint32_t Size) {

    uint32_t sizeToSkip = xTS::TS_PacketLength - Size;


    for (int i = 0; i < Size; i++) {
        m_Buffer[m_BufferSize + i] = Data[sizeToSkip + i];

    }
    m_BufferSize += Size;
}
