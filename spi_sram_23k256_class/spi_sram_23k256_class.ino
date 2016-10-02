#include <SPI.h>

//SRAM Status Register opcodes
#define BYTE_MODE B00000000                    // 0     dec
#define PAGE_MODE B10000000                    // 128   dec
#define SEQUENTIAL_MODE B01000000              // 64    dec

//SRAM Status Register opcodes disabling HOLD pin
#define BYTE_MODE_NO_HW_HOLD B00000001         // 1     dec
#define PAGE_MODE_NO_HW_HOLD B10000001         // 129   dec
#define SEQUENTIAL_MODE_NO_HW_HOLD B01000001   // 65    dec

//SRAM opcodes
#define READ_STATUS_REGISTER B00000101         // 5     dec
#define WRITE_STATUS_REGISTER B00000001        // 1     dec
#define READ B00000011                         // 3     dec
#define WRITE B00000010                        // 2     dec

#define ZERO ((uint8_t) 0x00)                  // 0     dec
#define MAX_VALUE ((uint8_t) 0xFF)             // 255   dec
#define MAX_ADDR ((uint16_t) 0x7FFF)           // 32767 dec
#define MAX_PAGE ((uint8_t) 0x0400)            // 1024  dec
#define MAX_WORD_PER_PAGE ((uint8_t) 0x20)     // 32    dec

#define SET_SS_LOW() (PORTB &= ~(1<<PORTB2)) //set SPI_SS low
#define SET_SS_HIGH() (PORTB |= (1<<PORTB2)) //set SPI_SS high

class SpiRAM {
  public :
    SpiRAM();
    ~SpiRAM();
    void setHwHold(bool hwHold);
    bool getHwHold();
    uint8_t readStatusRegister();
    void writeStatusRegister(uint8_t command);
    byte readPage(uint8_t page, uint8_t word_start, uint8_t* arr, byte size);
    byte writePage(uint8_t page, uint8_t word_start, const uint8_t* arr, byte size);
    byte readStream(uint16_t address, uint8_t* arr, byte size);
    byte writeStream(uint16_t address, const uint8_t* arr, byte size);
    uint8_t readByte(uint16_t address);
    void writeByte(uint16_t address, uint8_t data_byte);
    
  private :
    uint8_t mode;
    bool hwHold;
};

SpiRAM::SpiRAM() {
  this->mode = BYTE_MODE;
  this->hwHold = false;
}

SpiRAM::~SpiRAM() {
}

void SpiRAM::setHwHold(bool hwHold) {
  this->hwHold = hwHold;
}

bool SpiRAM::getHwHold() {
  return this->hwHold;
}

uint8_t SpiRAM::readStatusRegister() {
  uint8_t read_status;
  
  SET_SS_LOW();
  SPI.transfer(READ_STATUS_REGISTER);
  read_status = SPI.transfer(0xFF);
  SET_SS_HIGH();
  
  return read_status;
}

void SpiRAM::writeStatusRegister(uint8_t command) {
  this->mode = command;
  this->hwHold = (command & B00000001);
  
  SET_SS_LOW();
  SPI.transfer(WRITE_STATUS_REGISTER);
  SPI.transfer(command);
  SET_SS_HIGH();
}

/**
 *  Return how many bytes were written on selected page.
 *  -1 means error.
 */
byte SpiRAM::readPage(uint8_t page, uint8_t word_start, uint8_t* arr, byte size) {
  if(arr == NULL)
    return -1;
  
  if((this->mode != PAGE_MODE) && (this->mode != PAGE_MODE_NO_HW_HOLD))
    this->writeStatusRegister((this->hwHold == false) ? PAGE_MODE : PAGE_MODE_NO_HW_HOLD);
  
  byte i;
  
  SET_SS_LOW();
  SPI.transfer(READ);
  SPI.transfer((char) page);
  SPI.transfer((char) word_start);
  
  for(i = 0; i < size; i++) {
    arr[i] = SPI.transfer(0xFF);
  }
  SET_SS_HIGH();
  
  return i;
}

/**
 *  Return how many bytes were written on selected page.
 *  -1 means error.
 */
byte SpiRAM::writePage(uint8_t page, uint8_t word_start, const uint8_t* arr, byte size) {
  if(arr == NULL)
    return -1;
  
  if((this->mode != PAGE_MODE) && (this->mode != PAGE_MODE_NO_HW_HOLD))
    this->writeStatusRegister((this->hwHold == false) ? PAGE_MODE : PAGE_MODE_NO_HW_HOLD);
  
  byte i;
  
  SET_SS_LOW();
  SPI.transfer(WRITE);
  SPI.transfer((char) page);
  SPI.transfer((char) word_start);
  
  for(i = 0; i < size; i++) {
    SPI.transfer(arr[i]);
  }
  SET_SS_HIGH();
  
  return i;
}

/**
 *  Return how many bytes were written on selected page.
 *  -1 means error.
 */
byte SpiRAM::readStream(uint16_t address, uint8_t* arr, byte size) {
  if(arr == NULL)
    return -1;
  
  if((this->mode != SEQUENTIAL_MODE) && (this->mode != SEQUENTIAL_MODE_NO_HW_HOLD))
    this->writeStatusRegister((this->hwHold == false) ? SEQUENTIAL_MODE : SEQUENTIAL_MODE_NO_HW_HOLD);
  
  byte i;
  
  SET_SS_LOW();
  SPI.transfer(READ);
  SPI.transfer((char) (address >> 8));
  SPI.transfer((char) address);
  
  for(i = 0; i < size; i++) {
    arr[i] = SPI.transfer(0xFF);
  }
  SET_SS_HIGH();
  
  return i;
}

/**
 *  Return how many bytes were written on RAM as stream.
 *  -1 means error.
 */
byte SpiRAM::writeStream(uint16_t address, const uint8_t* arr, byte size) {
  if(arr == NULL)
    return -1;
  
  if((this->mode != SEQUENTIAL_MODE) && (this->mode != SEQUENTIAL_MODE_NO_HW_HOLD))
    this->writeStatusRegister((this->hwHold == false) ? SEQUENTIAL_MODE : SEQUENTIAL_MODE_NO_HW_HOLD);
    
  byte i;
  
  SET_SS_LOW();
  SPI.transfer(WRITE);
  SPI.transfer((char) (address >> 8));
  SPI.transfer((char) address);
  
  for(i = 0; i < size; i++) {
    SPI.transfer(arr[i]);
  }
  SET_SS_HIGH();
  
  return i;
}

uint8_t SpiRAM::readByte(uint16_t address) {
  if((this->mode != BYTE_MODE) && (this->mode != BYTE_MODE_NO_HW_HOLD))
    this->writeStatusRegister((this->hwHold == false) ? BYTE_MODE : BYTE_MODE_NO_HW_HOLD);
  
  uint8_t read_byte;
 
  SET_SS_LOW();
  SPI.transfer(READ);
  SPI.transfer((char) (address >> 8));
  SPI.transfer((char) address);
  read_byte = SPI.transfer(0xFF);
  SET_SS_HIGH();
  
  return read_byte;
}
 
void SpiRAM::writeByte(uint16_t address, uint8_t data_byte) {
  if((this->mode != BYTE_MODE) && (this->mode != BYTE_MODE_NO_HW_HOLD))
    this->writeStatusRegister((this->hwHold == false) ? BYTE_MODE : BYTE_MODE_NO_HW_HOLD);
    
  SET_SS_LOW();
  SPI.transfer(WRITE);
  SPI.transfer((char) (address >> 8));
  SPI.transfer((char) address);
  SPI.transfer(data_byte);
  SET_SS_HIGH();
}

SpiRAM ram = SpiRAM();

void setup(void) { 
  randomSeed(analogRead(0));
  
  Serial.begin(9600);
  SPI.begin();
  
  uint16_t addr;
  
  ram.writeStatusRegister(BYTE_MODE);
  
  if(ram.readStatusRegister() != BYTE_MODE) {
    Serial.println("Errory while setting status register (byte operative mode)!");
  }
  else {
    Serial.println("Byte operative mode correctly set!");
  }
  
  for (addr = 0; addr < (MAX_ADDR + 1); addr++) {
    ram.writeByte(addr, ZERO);
    
    if(ram.readByte(addr) != ZERO) {
      Serial.println("Error during initialize memory!");
    }
  }
  
  Serial.println("Memory initialize OK");
}

uint16_t addr = ZERO;
uint8_t rnd = ZERO;

#define BUFF_SIZE 32
uint8_t buff[BUFF_SIZE];
uint8_t buff_test[BUFF_SIZE];

void loop() {
  ram.writeStatusRegister(SEQUENTIAL_MODE);
  
  Serial.println("Switch to Sequential Mode");
  
  byte i;
  
  for(i = 0; i < BUFF_SIZE; i++)
    buff[i] = (uint8_t) random(ZERO, MAX_VALUE);
  
  ram.writeStream((uint16_t) 0x45FF, (uint8_t*) "Prova scrittura SRAM Spi!", (byte) 26);
  ram.readStream((uint16_t) 0x45FF, buff_test, (byte) 26);
  
  Serial.println((char*) buff_test);
  
  delay(10000);
}

void loop2() {
  ram.writeStatusRegister(PAGE_MODE);
  
  Serial.println("Switch to Page Mode");
  
  byte i;
  
  for(i = 0; i < BUFF_SIZE; i++)
    buff[i] = (uint8_t) random(ZERO, MAX_VALUE);
  
  ram.writePage((uint8_t) 1, (uint8_t) 0, buff, (byte) BUFF_SIZE);
  ram.readPage((uint8_t) 1, (uint8_t) 0, buff_test, (byte) BUFF_SIZE);

  for(i = 0; i < BUFF_SIZE; i++) {
    if(buff[i] != buff_test[i]) {
      Serial.println("Error!");
    }
    else {
      Serial.println("Test OK");
    }
  }
  
  delay(10000);
}

void loop3() {
  uint8_t read = ZERO;
  
  rnd = (uint8_t) random(ZERO, MAX_VALUE);
  
  ram.writeByte(addr, rnd);
  
  read = ram.readByte(addr);
  
  if(read != rnd) {
    Serial.println("An ERROR occurred when read/write memory to Spi RAM!");
    
    Serial.print("I write ");
    Serial.print(rnd);
    Serial.print(" and I read ");
    Serial.print(read);
    Serial.print(" to address ");
    Serial.print(addr);
    Serial.print("\r\n");
  }
  else {
    Serial.print(addr);
    Serial.print(" - ");
    Serial.println("Test OK");
  }
  
  addr = (addr + 1) % MAX_ADDR;    //L'indirizzo 32767 == 0x7FFF è l'ultimo valido!
  
  delay(10);
}

