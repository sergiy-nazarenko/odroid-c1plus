/*
  mcp_can.h
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee (loovee@seeed.cc)
  2014-1-16

  Contributor:

  Cory J. Fowler
  Latonita
  Woodward1
  Mehtajaghvi
  BykeBlast
  TheRo0T
  Tsipizic
  ralfEdmund
  Nathancheek
  BlueAndi
  Adlerweb
  Btetz
  Hurvajs
  ttlappalainen

  The MIT License (MIT)

  Copyright (c) 2013 Seeed Technology Inc.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#ifndef _MCP2515_H_
#define _MCP2515_H_

#include "mcp_can_dfs.h"

#define MAX_CHAR_IN_MESSAGE 8

// class MCP_CAN
// {
//     private:

    uint8_t   ext_flg;                         // identifier xxxID
                                            // either extended (the 29 LSB) or standard (the 11 LSB)
    uint16_t  can_id;                  // can id
    uint8_t   rtr;                             // rtr
    uint8_t   SPICS;
    SPIClass *pSPI;
    uint8_t   nReservedTx;                     // Count of tx buffers for reserved send
  	uint8_t   mcpMode;                         // Current controller mode

/*
*  mcp2515 driver function
*/

// private:

    // void mcp2515_reset(void);                                   // reset mcp2515

    uint8_t mcp2515_readRegister(const uint8_t address);              // read mcp2515's register

    void mcp2515_readRegisterS(const uint8_t address,
	                       uint8_t values[],
                               const uint8_t n);
    void mcp2515_setRegister(const uint8_t address,                // set mcp2515's register
                             const uint8_t value);

    void mcp2515_setRegisterS(const uint8_t address,               // set mcp2515's registers
                              const uint8_t values[],
                              const uint8_t n);

    void mcp2515_initCANBuffers(void);

    void mcp2515_modifyRegister(const uint8_t address,             // set bit of one register
                                const uint8_t mask,
                                const uint8_t data);

    uint8_t mcp2515_readStatus(void);                              // read mcp2515's Status
    uint8_t mcp2515_setCANCTRL_Mode(const uint8_t newmode);           // set mode
	uint8_t mcp2515_requestNewMode(const uint8_t newmode);                  // Set mode
    uint8_t mcp2515_configRate(const uint8_t canSpeed, const uint8_t clock);  // set baudrate
    uint8_t mcp2515_init(const uint8_t canSpeed, const uint8_t clock);   // mcp2515init

    void mcp2515_write_id( const uint8_t mcp_addr,                 // write can id
                               const uint8_t ext,
                               const uint16_t id );

    void mcp2515_read_id( const uint8_t mcp_addr,                  // read can id
                                    uint8_t* ext,
                                    uint16_t* id );

    void mcp2515_write_canMsg( const uint8_t buffer_sidh_addr, uint16_t id, uint8_t ext, uint8_t rtr, uint8_t len, volatile const uint8_t *buf);     // read can msg
    void mcp2515_read_canMsg( const uint8_t buffer_load_addr, volatile uint16_t *id, volatile uint8_t *ext, volatile uint8_t *rtr, volatile uint8_t *len, volatile uint8_t *buf);   // write can msg
    void mcp2515_start_transmit(const uint8_t mcp_addr);           // start transmit
    uint8_t mcp2515_getNextFreeTXBuf(uint8_t *txbuf_n);               // get Next free txbuf
    uint8_t mcp2515_isTXBufFree(uint8_t *txbuf_n, uint8_t iBuf);         // is buffer by index free

/*
*  can operator function
*/

    uint8_t mcp2515_sendMsg(uint16_t id, uint8_t ext, uint8_t rtrBit, uint8_t len, const uint8_t *buf, bool wait_sent=true); // send message

// public:
    MCP_CAN(uint8_t _CS=0);
    void mcp2515_init_CS(uint8_t _CS);                      // define CS after construction before begin()
    void mcp2515_setSPI(SPIClass *_pSPI) { pSPI=_pSPI; } // define SPI port to use before begin()
    void mcp2515_enableTxInterrupt(bool enable=true);    // enable transmit interrupt
    void mcp2515_reserveTxBuffers(uint8_t nTxBuf=0) { nReservedTx=(nTxBuf<MCP_N_TXBUFFERS?nTxBuf:MCP_N_TXBUFFERS-1); }
    uint8_t mcp2515_getLastTxBuffer() { return MCP_N_TXBUFFERS-1; } // read index of last tx buffer

    uint8_t mcp2515_begin(uint8_t speedset, const uint8_t clockset = MCP_16MHz);     // init can
    uint8_t mcp2515_init_Mask(uint8_t num, uint8_t ext, uint16_t ulData);       // init Masks
    uint8_t mcp2515_init_Filt(uint8_t num, uint8_t ext, uint16_t ulData);       // init filters
    void mcp2515_setSleepWakeup(uint8_t enable);                               // Enable or disable the wake up interrupt (If disabled the MCP2515 will not be woken up by CAN bus activity, making it send only)
	uint8_t mcp2515_sleep();													// Put the MCP2515 in sleep mode
	uint8_t mcp2515_wake();													// Wake MCP2515 manually from sleep
	uint8_t mcp2515_setMode(uint8_t opMode);                                      // Set operational mode
	uint8_t mcp2515_getMode();				                                    // Get operational mode
	uint8_t mcp2515_sendMsgBuf(uint16_t id, uint8_t ext, uint8_t rtrBit, uint8_t len, const uint8_t *buf, bool wait_sent=true);  // send buf
    uint8_t mcp2515_sendMsgBuf(uint16_t id, uint8_t ext, uint8_t len, const uint8_t *buf, bool wait_sent=true);               // send buf
    uint8_t mcp2515_readMsgBuf(uint8_t *len, uint8_t *buf);                          // read buf
    uint8_t mcp2515_readMsgBufID(uint16_t *ID, uint8_t *len, uint8_t *buf);     // read buf with object ID
    uint8_t mcp2515_checkReceive(void);                                        // if something received
    uint8_t mcp2515_checkError(void);                                          // if something error
    uint16_t mcp2515_getCanId(void);                                   // get can id when receive
    uint8_t mcp2515_isRemoteRequest(void);                                     // get RR flag when receive
    uint8_t mcp2515_isExtendedFrame(void);                                     // did we recieve 29bit frame?

    uint8_t mcp2515_readMsgBufID(uint8_t status, volatile uint16_t *id, volatile uint8_t *ext, volatile uint8_t *rtr, volatile uint8_t *len, volatile uint8_t *buf); // read buf with object ID
    uint8_t mcp2515_trySendMsgBuf(uint16_t id, uint8_t ext, uint8_t rtrBit, uint8_t len, const uint8_t *buf, uint8_t iTxBuf=0xff);  // as sendMsgBuf, but does not have any wait for free buffer
    uint8_t mcp2515_sendMsgBuf(uint8_t status, uint16_t id, uint8_t ext, uint8_t rtrBit, uint8_t len, volatile const uint8_t *buf); // send message buf by using parsed buffer status
    inline uint8_t mcp2515_trySendExtMsgBuf(uint16_t id, uint8_t len, const uint8_t *buf, uint8_t iTxBuf=0xff) {  // as trySendMsgBuf, but set ext=1 and rtr=0
      return mcp2515_trySendMsgBuf(id,1,0,len,buf,iTxBuf);
    }
    inline uint8_t mcp2515_sendExtMsgBuf(uint8_t status, uint16_t id, uint8_t len, volatile const uint8_t *buf) { // as sendMsgBuf, but set ext=1 and rtr=0
      return mcp2515_sendMsgBuf(status,id,1,0,len,buf);
    }
    void mcp2515_clearBufferTransmitIfFlags(uint8_t flags=0);                  // Clear transmit flags according to status
    uint8_t mcp2515_readRxTxStatus(void);                                      // read has something send or received
    uint8_t mcp2515_checkClearRxStatus(uint8_t *status);                          // read and clear and return first found rx status bit
    uint8_t mcp2515_checkClearTxStatus(uint8_t *status, uint8_t iTxBuf=0xff);        // read and clear and return first found or buffer specified tx status bit
	
	bool mcpPinMode(const uint8_t pin, const uint8_t mode);                  // switch supported pins between HiZ, interrupt, output or input
    bool mcpDigitalWrite(const uint8_t pin, const uint8_t mode);             // write HIGH or LOW to RX0BF/RX1BF
    uint8_t mcpDigitalRead(const uint8_t pin);                               // read HIGH or LOW from supported pins



#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
