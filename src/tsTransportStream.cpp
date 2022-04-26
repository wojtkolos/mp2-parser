#include "tsTransportStream.h"


//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================
void xTS_PacketHeader::Reset()
{
    header, SB, E, S, T, PID, TSC, AFC, CC = 0;
}

int32_t xTS_PacketHeader::Parse(const uint8_t* Input)
{
    this->SB = Input[0];                                                        //maski
    this->E = (Input[1] & 0x80) ? 1 : 0;                                        //128   10000000
    this->S = (Input[1] & 0x40) ? 1 : 0;                                        //64    01000000
    this->T = (Input[1] & 0x20) ? 1 : 0;                                        //32    00100000
    this->PID = From8To16((Input[1] & 0x1F), Input[2]);                         //31    00011111 + inp 2
    this->TSC = (Input[3] & 0xC0) >> 6;                                         //192   11000000 (6)>> 3 11
    this->AFC = (Input[3] & 0x30) >> 4;                                         //48    00110000 (4)>> 3 11
    this->CC = Input[3] & 0xF;                                                  //15    00001111
    return this->header = From8To32(Input[0], Input[1], Input[2], Input[3]);   // inp0 + inp1 + inp2 + inp3
    
}
void xTS_PacketHeader::Print() const
{
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
    return ((this->AFC == 2) || (this->AFC == 3))? true : false;
}

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================
void xTS_AdaptationField::Reset()
{
    AFC, AFL, DC, RA, SP, PCR, OR, SF, TP, EX = 0;
}

int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdaptationFieldControl)
{
    
    this->AFC = AdaptationFieldControl;
    if (this->AFC == 2 or this->AFC == 3) {
        this->AFL = Input[4];
        this->Stuffing = AFL - 1;
    }
    else {
        this->AFL = 0;
        this->Stuffing = 0;
    }
    this->DC = (Input[5] & 0x80) >> 7;
    this->RA = (Input[5] & 0x40) >> 6;
    this->SP = (Input[5] & 0x20) >> 5;
    this->PCR = (Input[5] & 0x10) >> 4;
    this->OR = (Input[5] & 0x8) >> 3;
    this->SF = (Input[5] & 0x4) >> 2;
    this->TP = (Input[5] & 0x2) >> 1;
    this->EX = Input[5] & 0x1;

    int pointer = 6;
    //check if has PCR flag
    if (this->PCR) {
        this->PCR = From8To64(0,0,0, Input[pointer], Input[++pointer], Input[++pointer], Input[++pointer], Input[++pointer]);
        this->PCR_base >>= 7;

        this->PCR_reserved = (Input[pointer] & 0x7E);
        this->PCR_reserved >>= 1;
        this->PCR_extension = From8To16((Input[pointer] & 0x1),Input[++pointer]);
        this->Stuffing -= 6;
    }
    if (this->OR) {
        this->OPCR_base = From8To64(0, 0, 0, Input[pointer], Input[++pointer], Input[++pointer], Input[++pointer], Input[++pointer]);
        this->OPCR_base >>= 7;

        this->OPCR_reserved = (Input[pointer] & 0x7E);
        this->OPCR_reserved >>= 1;
        this->OPCR_extension = From8To16((Input[pointer] & 0x1), Input[++pointer]);
        this->Stuffing -= 6;
    }
    return From8To32(this->AFL, Input[5]);
}

void xTS_AdaptationField::Print() const
{
    printf("AFL : ");
    printf("AFL=%d ", this->AFL);
    printf("DC=%d ", this->DC);
    printf("RA=%d ", this->RA);
    printf("SP=%d ", this->SP);
    printf("PR=%d ", this->PCR);
    printf("OR=%d ", this->OR);
    printf("SF=%d ", this->SF);
    printf("TP=%d ", this->TP);
    printf("EX=%d ", this->TP);

    if(this->PCR){ 
        printf("\n\n\nPCR=%d ", (this->PCR_base * xTS::BaseToExtendedClockMultiplier + this->PCR_extension)); }
    if (this->OR) { 
        printf("\n\nOPCR=%d ", (this->OPCR_base * xTS::BaseToExtendedClockMultiplier + this->OPCR_extension)); }
}

//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================
void xPES_PacketHeader::Reset()
{
    this->m_PacketStartCodePrefix, this->m_StreamId, this->m_PacketLength = 0;
}

int32_t xPES_PacketHeader::Parse(const uint8_t* Input)
{
    this->m_PacketStartCodePrefix = From8To24(Input[0], Input[1], Input[2]);
    this->m_StreamId = Input[3];
    this->m_PacketLength = From8To16(Input[4], Input[5]);

    if (!this->m_HeaderLength) { this->m_HeaderLength = 6; }

    return this->m_PacketStartCodePrefix;
}

void xPES_PacketHeader::Print() const
{
    //PES: PSCP = 1 SID = 192 L = 2888
    printf("PES: ");
    printf("PSCP=%d ", this->m_PacketStartCodePrefix);
    printf("SID=%d ", this->m_StreamId);
    printf("L=%d ", this->m_PacketLength);
}

//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================
xPES_Assembler::xPES_Assembler() { }

xPES_Assembler::~xPES_Assembler() { delete this->m_Buffer; }

void xPES_Assembler::Init(int32_t PID)
{
    this->m_PID = PID;
    this->m_Buffer = new uint8_t[100000];
    this->m_BufferSize = 0;
    this->m_LastContinuityCounter = 0;
    this->m_Started = false;
}

xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField)
{
    if(this->m_PID != PacketHeader->getPID()) {
        return eResult::UnexpectedPID;
    } 
    else if(this->m_Started || PacketHeader->getContinuityCounter() == 0) {
        if (PacketHeader->getContinuityCounter() == this->m_LastContinuityCounter) {

            uint8_t TS_AdaptationLenght = 0;
            if (PacketHeader->hasAdaptationField()) {
                TS_AdaptationLenght = AdaptationField->getNumBytes();
            }
            this->m_LastContinuityCounter++;
            if (this->m_LastContinuityCounter < 15) this->m_LastContinuityCounter %= 16;
            uint32_t tempSize = xTS::TS_PacketLength - xTS::TS_HeaderLength - TS_AdaptationLenght;

            if (PacketHeader->getPayloadUnitStartIndicator()) {
                this->m_Started = true;

                uint32_t sizeToSkip = xTS::TS_PacketLength - tempSize;
                uint8_t* firstPacketBuffer = new uint8_t[188];

                for (int i = 0; i < tempSize; i++) {
                    firstPacketBuffer[i] = TransportStreamPacket[sizeToSkip + i];
                }
                this->m_PESH.Parse(firstPacketBuffer);

                tempSize -= getHeaderLenght();

                xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
                return eResult::AssemblingStarted;
            }
            else if (PacketHeader->hasAdaptationField()) {
                xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
                this->m_Started = false;
                this->m_LastContinuityCounter = 0;
                return eResult::AssemblingFinished;
            }
            else {
                xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
                return eResult::AssemblingContinue;
            }
        }
    }
    this->m_Started = false;
    return eResult::StreamPackedLost;
}

void xPES_Assembler::xBufferReset()
{
    this->m_PESH.Reset();
    this->m_BufferSize = 0;
}

void xPES_Assembler::xBufferAppend(const uint8_t* data, uint32_t size)
{
    uint32_t sizeToSkip = xTS::TS_PacketLength - size;

    for (int i = 0; i < size; i++) {
        this->m_Buffer[this->m_BufferSize + i] = data[sizeToSkip + i];
    }
    this->m_BufferSize += size;
}

void xPES_Assembler::assemblerPes136(const uint8_t* TS_PacketBuffer, const xTS_PacketHeader* TS_PacketHeader, const xTS_AdaptationField* TS_AdaptationField) {

    xPES_Assembler::eResult result = this->AbsorbPacket(TS_PacketBuffer, TS_PacketHeader, TS_AdaptationField);
    switch (result)
    {
        case xPES_Assembler::eResult::StreamPackedLost: printf("PcktLost \n"); break;
        case xPES_Assembler::eResult::UnexpectedPID: printf("UnexpectedPID \n"); break;
        case xPES_Assembler::eResult::AssemblingStarted: printf("Started "); this->PrintPESH(); break;
        case xPES_Assembler::eResult::AssemblingContinue: printf("Continue \n"); break;
        case xPES_Assembler::eResult::AssemblingFinished: printf("Finished \n"); printf("PES: Len=%d", this->getNumPacketBytes()); break;
        default: break;
    }
}
//=============================================================================================================================================================================



