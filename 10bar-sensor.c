/*
🧮 Birim Dönüşümleri:
1 psi ≈ 0.06895 bar
150 psi ≈ 10.34 bar
spi = Pounds per Square Inch
*/
char buffer[64];

void UART3_Init_Config(unsigned long baudrate) {
    PPS_Mapping(69, _INPUT,  _U3RX);
    PPS_Mapping(68, _OUTPUT, _U3TX);
    UART3_Init(baudrate);
    Delay_ms(100);
}

void SPI1_PPS_Init() {
    PPS_Mapping(96, _OUTPUT, _SCK1OUT);
    PPS_Mapping(97, _OUTPUT, _SDO1);
    PPS_Mapping(98, _INPUT, _SDI1);
    PPS_Mapping(104, _OUTPUT, _SS1OUT);

    TRISF.F0 = 0;
    TRISF.F1 = 0;
    TRISF.F2 = 1;
    TRISF.F8 = 0;
    LATF.F8 = 1;
}

void SPI1_Initialize_Master() {
    SPI1_Init();
}

unsigned int Read_ADC_Channel(unsigned char channel) {
    unsigned char msb, lsb;
    unsigned int adcValue;
    unsigned char command;
    unsigned char channelBits = channel << 6;

    if (channel > 3)
        command = 0b00000111;
    else
        command = 0b00000110;

    LATF.F8 = 0;
    SPI1_Write(command);
    msb = SPI1_Read(channelBits);
    lsb = SPI1_Read(0x00);
    LATF.F8 = 1;

    msb = msb & 0b00001111;
    adcValue = (msb << 8) | lsb;
    return adcValue;
}

void Read_Pressure() {
    unsigned int adcRaw;
    float voltage, pressure_psi, pressure_bar;
    char txt[128];

    // Ayarlanabilir parametreler
    float Vsup = 5.0;        // Sensör beslemesi
    float Pmin = 0.0;        // Min. basinç (psi)
    float Pmax = 150.0;      // Max. basinç (psi)

    // ADC verisi okunuyor
    adcRaw = Read_ADC_Channel(0);  // CH0
    voltage = (adcRaw / 4095.0) * Vsup;  // ADC -> Voltaj

    // Basinç hesaplama (psi cinsinden)
    pressure_psi = ((voltage - 0.1 * Vsup) / (0.8 * Vsup)) * (Pmax - Pmin) + Pmin;

    // psi -> bar
    pressure_bar = pressure_psi * 0.06895;

    // UART ile yazdir
    sprintf(txt,
        "ADC: %u\r\nVoltage: %.3f V\r\nPressure: %.2f psi / %.3f bar\r\n\r\n",
        adcRaw, voltage, pressure_psi, pressure_bar);

    UART3_Write_Text(txt);
    Delay_ms(1000);
}



void main() {
    PLLFBD = 70;
    CLKDIV = 0x0000;

    SPI1_PPS_Init();
    SPI1_Initialize_Master();
    UART3_Init_Config(9600);

    UART3_Write_Text("Okuma Basladi...\r\n");

    while (1) {
        Read_Pressure();
    }
}
