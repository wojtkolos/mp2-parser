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
    printf("\x1B[36mTS: SB=%d E=%d S=%d T=%d \x1B[33mPID=%d\033[0m \x1B[36mTSC=%d AFC=%d CC=%d\033[0m", 
        this->SB, this->E, this->S, this->T, this->PID, 
        this->TSC, this->AFC, this->CC);
}

bool xTS_PacketHeader::hasAdaptationField() const {
    return ((this->AFC == 2) || (this->AFC == 3))? true : false;
}

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================
void xTS_AdaptationField::Reset()
{
    AFC, AF, DC, RA, SP, PCR, OR, SF, TP, EX = 0;
}

int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdaptationFieldControl)
{
    
    this->AFC = AdaptationFieldControl;
    if (this->AFC == 2 or this->AFC == 3) {
        this->AF = Input[4];
        this->Stuffing = AF - 1;
    }
    else {
        this->AF = 0;
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
    return From8To32(this->AF, Input[5]);
}

void xTS_AdaptationField::Print() const
{
    printf("\x1B[34mAFL: AF=%d DC=%d RA=%d SP=%d PR=%d OR=%d SF=%d TP=%d EX=%d\033[0m\n",
        this->AF, this->DC, this->RA, this->SP,
        this->PCR, this->OR, this->SF, this->TP, this->TP);


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
    this->m_PacketStartCodePrefix, this->m_StreamId, this->m_PacketLength, 
        this->PTS_DTS_flags = 0, this->PTS, this->DTS = 0;
}

int32_t xPES_PacketHeader::Parse(const uint8_t* Input)
{
    this->m_PacketStartCodePrefix = From8To24(Input[0], Input[1], Input[2]);
    this->m_StreamId = Input[3];
    this->m_PacketLength = From8To16(Input[4], Input[5]);
    if (!this->m_HeaderLength) { this->m_HeaderLength = 6; }

    //0xBD
    //0xC0 - 0xDF
    //0xE0 - 0xEF

    //no fo: 0xBE,0xBF
    if (this->m_StreamId == 0xBD || (this->m_StreamId >= 0xC0 && this->m_StreamId <= 0xEF)) {
    /*    if(
        m_StreamId != 0xBC &&
            m_StreamId != 0xBE &&
            m_StreamId != 0xBF &&
            m_StreamId != 0xF0 &&
            m_StreamId != 0xF1 &&
            m_StreamId != 0xFF &&
            m_StreamId != 0xF2 &&
            m_StreamId != 0xF8){*/
            this->m_HeaderLength = 9;
        
        this->PTS_DTS_flags = (Input[7] & 0xC0) >> 6;

        if ((Input[7] & 0xC0) == 0xC0) {
            this->m_HeaderLength += 10;

            this->PTS = Input[9] & 0xE;
            this->PTS <<= 7;
            this->PTS += Input[10];
            this->PTS <<= 8;
            this->PTS += Input[11] & 0xFE;
            this->PTS <<= 7;
            this->PTS += Input[12];
            this->PTS <<= 7;
            this->PTS += Input[13] >> 1;


            this->DTS = Input[14] & 0xE;
            this->DTS <<= 7;
            this->DTS += Input[15];
            this->DTS <<= 8;
            this->DTS += Input[16] & 0xFE;
            this->DTS <<= 7;
            this->DTS += Input[17];
            this->DTS <<= 7;
            this->DTS += Input[18] >> 1;
        }
        else if ((Input[7] & 0x80) == 0x80) {
            this->m_HeaderLength += 5;

            this->PTS = Input[9] & 0xE;
            this->PTS <<= 7;
            this->PTS += Input[10];
            this->PTS <<= 8;
            this->PTS += Input[11] & 0xFE;
            this->PTS <<= 7;
            this->PTS += Input[12];
            this->PTS <<= 7;
            this->PTS += Input[13] >> 1;

        }

        //ESCR_flag == '1'
        if (Input[7] & 0x20) {
            this->m_HeaderLength += 6;
        }
        //ES_rate_flag == '1'
        if (Input[7] & 0x10) {
            this->m_HeaderLength += 3;
        }
        //DSM_trick_mode_flag == '1'
        if (Input[7] & 0x8) {
            this->m_HeaderLength += 0;
        }
        //additional_copy_info_flag == '1'
        if (Input[7] & 0x4) {
            this->m_HeaderLength += 1;
        }
        //PES_CRC_flag == '1')
        if (Input[7] & 0x2) {
            this->m_HeaderLength += 2;
        }
        //PES_extension_flag == '1'
        if (Input[7] & 0x1) {
            int point = this->m_HeaderLength;
            this->m_HeaderLength += 1;
            //PES_private_data_flag == '1' -> 128
            if (Input[point] & 0x80) {
                this->m_HeaderLength += 16;
            }
            //pack_header_field_flag == '1' -> 8
            if (Input[point] & 0x40) {
                this->m_HeaderLength += 1;
            }
            //program packet sequence counter flag
            if (Input[point] & 0x20) {
                this->m_HeaderLength += 2;
            }
            //P-STD buffer flag
            if (Input[point] & 0x10) {
                this->m_HeaderLength += 2;
            }
            //P-STD buffer flag
            if (Input[point] & 0x1) {
                this->m_HeaderLength += 2;
            }
        }
    }

    return this->m_PacketStartCodePrefix;
}

void xPES_PacketHeader::Print() const
{
    //PES: PSCP = 1 SID = 192 L = 2888
    printf("PES: ");
    printf("PSCP=%d ", this->m_PacketStartCodePrefix);
    printf("SID=%d ", this->m_StreamId);
    printf("L=%d\n", this->m_PacketLength);

    if (PTS_DTS_flags == 0b10)
        printf("PTS=%d ", this->PTS);

    if (PTS_DTS_flags == 0b11) {
        printf("PTS=%d ", this->PTS);
        printf("DTS=%d ", this->DTS);
    }
}

//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================
xPES_Assembler::xPES_Assembler() { }

xPES_Assembler::~xPES_Assembler() { delete[] this->m_Buffer; }

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

/*
xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField)
{
    if(this->m_PID != PacketHeader->getPID()) {
        return eResult::UnexpectedPID;
    } 

    if (this->m_BufferSize > 100000) { return eResult::BufferFull; }

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
                memset(firstPacketBuffer, 0, sizeof(firstPacketBuffer));

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
                this->m_LastContinuityCounter = PacketHeader->getContinuityCounter();
                return eResult::AssemblingFinished;
            }
            else {
                xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
                return eResult::AssemblingContinue;
            }
        }
    }
    if (PacketHeader->hasAdaptationField()) {
        this->m_LastContinuityCounter = PacketHeader->getContinuityCounter();
        this->m_Started = false;
        return eResult::AssemblingFinished;
    }
    return eResult::StreamPackedLost;
}
*/
void xPES_Assembler::xBufferReset()
{
    this->m_PESH.Reset();
    memset(m_Buffer, 0, sizeof(m_Buffer));
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

void xPES_Assembler::assemblerPes(const uint8_t* TS_PacketBuffer, const xTS_PacketHeader* TS_PacketHeader, 
    const xTS_AdaptationField* TS_AdaptationField, FILE* File) {

    xPES_Assembler::eResult result = this->AbsorbPacket(TS_PacketBuffer, TS_PacketHeader, TS_AdaptationField);
    switch (result)
    {
        case xPES_Assembler::eResult::AssemblingStarted: {
            printf("\x1B[35mAssembling Started\033[0m\t\t\n"); 
            this->PrintPESH(); 
            break;
        }
        case xPES_Assembler::eResult::AssemblingContinue: {
            printf("\x1B[35mAssembling Continue\033[0m\t\t\n"); 
            break; 
        }
        case xPES_Assembler::eResult::AssemblingFinished: {
            printf("\x1B[35mAssembling Finished\033[0m\t\t\n"); 
            printf("PES: PcktLen=%d HeadLen=%d DataLen=%d\n", this->getNumPacketBytes(), this->getHeaderLenght(), this->getNumPacketBytes() - this->getHeaderLenght());
            saveBufferToFile(File);
            break;
        }
        case xPES_Assembler::eResult::StreamPackedLost: {
            printf("\x1B[31mPacket Lost\033[0m\t\t\n");
            break;
        }
        case xPES_Assembler::eResult::UnexpectedPID: {
            printf("\x1B[31mUnexpected PID\033[0m\t\t\n");
            break;
        }
        case xPES_Assembler::eResult::BufferFull: {
            printf("\x1B[32mBuffer Full!!!!\033[0m\t\t\n");
        }
        default: break;
    }
}

void xPES_Assembler::saveBufferToFile(FILE* AudioMP2)
{
    fwrite(this->getPacket(), 1, this->getNumPacketBytes(), AudioMP2);
    this->xBufferReset();
}
//=============================================================================================================================================================================



