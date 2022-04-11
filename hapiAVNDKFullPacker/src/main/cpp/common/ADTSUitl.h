
int samplingFrequencyIndex(int samplingFrequency) {

    int ret = 0;
    switch (samplingFrequency) {
        case 96000 :
            ret = 0;
            break;
        case 88200 :
            ret = 1;
            break;
        case 64000 :
            ret = 2;
            break;
        case 48000 :
            ret = 3;
            break;
        case 44100 :
            ret = 4;
            break;
        case 32000 :
            ret = 5;
            break;
        case 24000 :
            ret = 6;
            break;
        case 22050 :
            ret = 7;
            break;
        case 16000 :
            ret = 8;
            break;
        case 12000 :
            ret = 9;
            break;
        case 11025 :
            ret = 10;
            break;
        case 8000 :
            ret = 11;
            break;
        case 7350 :
            ret = 12;
            break;
        default: ret = 15;
            break;
    }
    return ret;
}

int channelConfiguration(int channelCount
) {
    int ret = 0;
    switch (channelCount) {
        case 1  :
            ret = 1;
            break;
        case 2 :
            ret = 2;
            break;
        case 3 :
            ret = 3;
            break;
        case 4 :
            ret = 4;
            break;
        case 5 :
            ret = 5;
            break;
        case 6 :
            ret = 6;
            break;
        case 8 :
            ret = 7;
            break;
        default: ret = 0;
            break;
    }
    return ret;
}


void addADTS(unsigned char *packet, int len, int samplingFrequency, int channelCount) {
    int profile = 2; // AAC LC
    int freqIdx = samplingFrequencyIndex(samplingFrequency); // 44.1KHz
    int chanCfg = channelConfiguration(channelCount); // CPE
    int packetLen = len + 7;//ADTS头加AAC数据总长度
    packet[0] = (unsigned char) 0xFF;
    packet[1] = (unsigned char) 0xF9;
    packet[2] = (unsigned char) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
    packet[3] = (unsigned char) (((chanCfg & 3) << 6) + (packetLen >> 11));
    packet[4] = (unsigned char) ((packetLen & 0x7FF) >> 3);
    packet[5] = (unsigned char) (((packetLen & 7) << 5) + 0x1F);
    packet[6] = (unsigned char) 0xFC;

}

