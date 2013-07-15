#pragma once

#include "RS9110_UART_Test.h"

#include "RS9110_UART.h"
#include "PersistorWin32Mock.h"

#include <cstdio>
#include <cppunit\config\SourcePrefix.h>



void RS9110_UART_Test::setUp ()
{
    mockFile    = new PersistorWin32Mock();
    rs          = new RS9110_UART(mockFile);
}


void RS9110_UART_Test::tearDown ()
{
    delete mockFile;
    delete rs;
}


CPPUNIT_TEST_SUITE_REGISTRATION(RS9110_UART_Test);


void RS9110_UART_Test::CompareStream (const char *stream, unsigned int size)
{
    char *expected  = (char *) stream;
    char *actual    = mockFile->GetBufferData();

    //printf("%s (%d) \n%s (%d)", mockFile->GetBufferData(), mockFile->GetBufferSize(), stream, strlen(stream));

    int rtn;
    if(size == 0)
    {
        CPPUNIT_ASSERT(strlen(stream) == mockFile->GetBufferSize());
        rtn = strcmp(expected, actual);
    }
    else
    {
        CPPUNIT_ASSERT(size == mockFile->GetBufferSize());
        rtn = memcmp(expected, actual, size);
    }
   
    CPPUNIT_ASSERT(rtn == 0);
}


void RS9110_UART_Test::ProcessMessageTest ()
{
    bool bRtn;
    int respLen;


    bRtn = rs->ProcessMessage("OK\r\n", 4);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_OK);
    CPPUNIT_ASSERT(rs->GetErrorCode() == RS9110_UART::ERROR_NONE);
    rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == 0);

    bRtn = rs->ProcessMessage("ERROR\x87\r\n", 8);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_ERROR);
    CPPUNIT_ASSERT(rs->GetErrorCode() == RS9110_UART::ERROR_NO_EXIST_TCP_SERVER);
    rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == 0);

    bRtn = rs->ProcessMessage("AT+RSI_READ\x0E\x00\xC0\xA8\x01\x01\x41\x1F abcd\r\n", 26);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_READ);
    rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == 13);
    
    bRtn = rs->ProcessMessage("AT+RSI_CLOSE\r\n", 14);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_CLOSE);
    rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == 0);

    bRtn = rs->ProcessMessage("SLEEP\r\n", 7);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_SLEEP);
    rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == 0);

    bRtn = rs->ProcessMessage("UNKNOWN\r\n", 9);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == -1);
}


void RS9110_UART_Test::GetResponseTest ()
{
    bool bRtn;
    int  respLen;


    /* GetNumScanResults */
    RS9110_UART::TNumScanResults *numScanResults;
    bRtn = rs->ProcessMessage("OK\x0A\r\n", 5);
    CPPUNIT_ASSERT(bRtn == true);
    numScanResults = (RS9110_UART::TNumScanResults *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == 1);
    CPPUNIT_ASSERT(numScanResults->value == 10);

    /* Scan */
    RS9110_UART::TScan *scan;
    bRtn = rs->ProcessMessage("OKRedpine_net1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x14Redpine_net2\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x0E\r\n", 72);
    CPPUNIT_ASSERT(bRtn == true);
    scan = (RS9110_UART::TScan *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == (sizeof(RS9110_UART::TScan) * 2));
    CPPUNIT_ASSERT(strcmp(scan->ssid, "Redpine_net1") == 0);
    CPPUNIT_ASSERT(scan->mode == RS9110_UART::SEC_MODE_WPA2);
    CPPUNIT_ASSERT(scan->rssi == 0x14);
    scan++;
    CPPUNIT_ASSERT(strcmp(scan->ssid, "Redpine_net2") == 0);
    CPPUNIT_ASSERT(scan->mode == RS9110_UART::SEC_MODE_WPA2);
    CPPUNIT_ASSERT(scan->rssi == 0x0E);

    /* GetMACOfAPs */
    RS9110_UART::TBssid *bssid;
    bRtn = rs->ProcessMessage("OKRedpine_net\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x23\xA7\x1B\x8D\x31\r\n", 42);
    CPPUNIT_ASSERT(bRtn == true);
    bssid = (RS9110_UART::TBssid *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == sizeof(RS9110_UART::TBssid));
    CPPUNIT_ASSERT(strcmp(bssid->ssid, "Redpine_net") == 0);
    CPPUNIT_ASSERT(memcmp(bssid->bssid, "\x00\x23\xA7\x1B\x8D\x31", sizeof(bssid->bssid)) == 0);

    /* GetNetworkType */
    RS9110_UART::TNetworkType *networkType;
    bRtn = rs->ProcessMessage("OKRedpine_net\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\r\n", 37);
    CPPUNIT_ASSERT(bRtn == true);
    networkType = (RS9110_UART::TNetworkType *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == sizeof(RS9110_UART::TNetworkType));
    CPPUNIT_ASSERT(strcmp(networkType->ssid, "Redpine_net") == 0);
    CPPUNIT_ASSERT(networkType->nwType == RS9110_UART::NW_TYPE_IBSS);

    /* IPConfiguration */
    RS9110_UART::TIPConfig *ipConfig;
    bRtn = rs->ProcessMessage("OK\x00\x23\xA7\x1B\x8D\x31\x01\x02\x03\x04\x05\x06\x07\x08\x09\x10\x11\x12\r\n", 22);
    CPPUNIT_ASSERT(bRtn == true);
    ipConfig = (RS9110_UART::TIPConfig *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == sizeof(RS9110_UART::TIPConfig));
    CPPUNIT_ASSERT(memcmp(ipConfig->mac,     "\x00\x23\xA7\x1B\x8D\x31", sizeof(ipConfig->mac))     == 0);
    CPPUNIT_ASSERT(memcmp(ipConfig->address, "\x01\x02\x03\x04",         sizeof(ipConfig->address)) == 0);
    CPPUNIT_ASSERT(memcmp(ipConfig->subnet,  "\x05\x06\x07\x08",         sizeof(ipConfig->subnet))  == 0);
    CPPUNIT_ASSERT(memcmp(ipConfig->gateway, "\x09\x10\x11\x12",         sizeof(ipConfig->gateway)) == 0);

    /* OpenTcpSocket and all socket openers */
    RS9110_UART::TSocket *socket;
    bRtn = rs->ProcessMessage("OK\x01\r\n", 5);
    CPPUNIT_ASSERT(bRtn == true);
    socket = (RS9110_UART::TSocket *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == sizeof(RS9110_UART::TSocket));
    CPPUNIT_ASSERT(socket->id == 0x01);

    /* GetSocketStatus */
    RS9110_UART::TSocketStatus *socketStatus;
    bRtn = rs->ProcessMessage("OK\x07\xC0\xA8\x28\x0A\x41\x1F\r\n", 11);
    CPPUNIT_ASSERT(bRtn == true);
    socketStatus = (RS9110_UART::TSocketStatus *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == sizeof(RS9110_UART::TSocketStatus));
    CPPUNIT_ASSERT(socketStatus->id == 0x07);
    CPPUNIT_ASSERT(memcmp(socketStatus->address, "\xC0\xA8\x28\x0A", sizeof(socketStatus->address)) == 0);
    CPPUNIT_ASSERT(socketStatus->port == 0x1F41);

    /* Incoming data */
    RS9110_UART::TReadUDP readUDP;
    bRtn = rs->ProcessMessage("AT+RSI_READ\x01\x05\x00\xC0\xA8\x01\x01\x41\x1F abcd\r\n", 27);
    CPPUNIT_ASSERT(bRtn == true);
    rs->Read(readUDP);
    CPPUNIT_ASSERT(readUDP.socketId == 0x01);
    CPPUNIT_ASSERT(readUDP.size == 0x0005);
    CPPUNIT_ASSERT(memcmp(readUDP.address, "\xC0\xA8\x01\x01", 4) == 0);
    CPPUNIT_ASSERT(readUDP.srcPort == 0x1F41);
    CPPUNIT_ASSERT(memcmp(readUDP.data, " abcd", readUDP.size) == 0);

    RS9110_UART::TReadTCP readTCP;
    bRtn = rs->ProcessMessage("AT+RSI_READ\x01\x05\x00 abcd\r\n", 21);
    CPPUNIT_ASSERT(bRtn == true);
    rs->Read(readTCP);
    CPPUNIT_ASSERT(readTCP.socketId == 0x01);
    CPPUNIT_ASSERT(readTCP.size == 0x0005);
    CPPUNIT_ASSERT(memcmp(readTCP.data, " abcd", readTCP.size) == 0);

    /* GetDNS */
    RS9110_UART::TDNSGet *dnsGet;
    bRtn = rs->ProcessMessage("OK\x03\x0C\xA8\x0A\x0A\xC0\xA8\x0A\x14\xC0\xA8\x0A\x1E\r\n", 17);
    CPPUNIT_ASSERT(bRtn == true);
    dnsGet = (RS9110_UART::TDNSGet *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(dnsGet->numIPs == 0x03);
    CPPUNIT_ASSERT(memcmp(dnsGet->address[0], "\x0C\xA8\x0A\x0A", sizeof(dnsGet->address[0])) == 0);
    CPPUNIT_ASSERT(memcmp(dnsGet->address[1], "\xC0\xA8\x0A\x14", sizeof(dnsGet->address[0])) == 0);
    CPPUNIT_ASSERT(memcmp(dnsGet->address[2], "\xC0\xA8\x0A\x1E", sizeof(dnsGet->address[0])) == 0);

    /* GetFirmwareVersion */
    RS9110_UART::TFWVersion *fwVersion;
    bRtn = rs->ProcessMessage("OK4.7.1\r\n", 9);
    CPPUNIT_ASSERT(bRtn == true);
    fwVersion = (RS9110_UART::TFWVersion *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == sizeof(RS9110_UART::TFWVersion));
    CPPUNIT_ASSERT(memcmp(fwVersion->version, "4.7.1", sizeof(fwVersion->version)) == 0);

    /* GetNetworkParameters */
    RS9110_UART::TNetworkParams *nwParams;
    bRtn = rs->ProcessMessage("OKRedpine_net\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01Redpine_net_password\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x01\x02\x03\x04\x05\x06\x01\xC0\xA8\x32\x02\xC0\xA8\x32\x01\xFF\xFF\xFF\x00\x02\x01\x00\x1F\x41\x41\x1F\x01\x02\x03\x04\x02\x04\x1A\xF1\x41\x4C\x05\x06\x07\x08\r\n", 110);
    CPPUNIT_ASSERT(bRtn == true);
    nwParams = (RS9110_UART::TNetworkParams *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == 106);
    CPPUNIT_ASSERT(strcmp(nwParams->ssid, "Redpine_net") == 0);
    CPPUNIT_ASSERT(nwParams->secMode == RS9110_UART::SEC_MODE_WPA);
    CPPUNIT_ASSERT(strcmp(nwParams->psk, "Redpine_net_password") == 0);
    CPPUNIT_ASSERT(nwParams->channel == RS9110_UART::CHANNEL_24_6);
    CPPUNIT_ASSERT(memcmp(nwParams->mac, "\x01\x02\x03\x04\x05\x06", sizeof(nwParams->mac)) == 0);
    CPPUNIT_ASSERT(nwParams->dhcpMode == RS9110_UART::DHCP_DHCP);
    CPPUNIT_ASSERT(memcmp(nwParams->address, "\xC0\xA8\x32\x02", sizeof(nwParams->address)) == 0);
    CPPUNIT_ASSERT(memcmp(nwParams->subnet,  "\xC0\xA8\x32\x01", sizeof(nwParams->subnet))  == 0);
    CPPUNIT_ASSERT(memcmp(nwParams->gateway, "\xFF\xFF\xFF\x00", sizeof(nwParams->gateway)) == 0);
    CPPUNIT_ASSERT(nwParams->numOpenSockets == 0x02);
    CPPUNIT_ASSERT(nwParams->socketDetails[0].id == 0x01);
    CPPUNIT_ASSERT(nwParams->socketDetails[0].type == RS9110_UART::SOCKET_TCP);
    CPPUNIT_ASSERT(nwParams->socketDetails[0].srcPort == 0x411F);
    CPPUNIT_ASSERT(nwParams->socketDetails[0].dstPort == 0x1F41);
    CPPUNIT_ASSERT(memcmp(nwParams->socketDetails[0].dstAddress, "\x01\x02\x03\x04", sizeof(nwParams->socketDetails[0].dstAddress)) == 0);
    CPPUNIT_ASSERT(nwParams->socketDetails[1].id == 0x02);
    CPPUNIT_ASSERT(nwParams->socketDetails[1].type == RS9110_UART::SOCKET_LUDP);
    CPPUNIT_ASSERT(nwParams->socketDetails[1].srcPort == 0xF11A);
    CPPUNIT_ASSERT(nwParams->socketDetails[1].dstPort == 0x4C41);
    CPPUNIT_ASSERT(memcmp(nwParams->socketDetails[1].dstAddress, "\x05\x06\x07\x08", sizeof(nwParams->socketDetails[1].dstAddress)) == 0);

    /* GetMACAddress */
    RS9110_UART::TMACAddress *mac;
    bRtn = rs->ProcessMessage("OK\x00\x23\xA7\x1B\x8D\x31\r\n", 10);
    CPPUNIT_ASSERT(bRtn == true);
    mac = (RS9110_UART::TMACAddress *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == sizeof(RS9110_UART::TMACAddress));
    CPPUNIT_ASSERT(memcmp(mac->mac, "\x00\x23\xA7\x1B\x8D\x31", sizeof(mac->mac)) == 0);

    /* GetRSSI */
    RS9110_UART::TRSSI *rssi;
    bRtn = rs->ProcessMessage("OK\x14\r\n", 5);
    CPPUNIT_ASSERT(bRtn == true);
    rssi = (RS9110_UART::TRSSI *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(respLen == sizeof(RS9110_UART::TRSSI));
    CPPUNIT_ASSERT(rssi->value == 0x14);

    /* GetConfiguration */
    RS9110_UART::TStoredConfig *config;
    bRtn = rs->ProcessMessage("OK\x00\x01\x06\x00\x02\x04\x02 Redpine_net_password\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00 Redpine_net\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x01\x02\x03\x04\x05\x06\x07\x08\x09\x10\x11\x12\xAA\xBB\xCC\xDD\r\n", 127);
    CPPUNIT_ASSERT(bRtn == true);
    config = (RS9110_UART::TStoredConfig *) rs->GetResponse(respLen);
    CPPUNIT_ASSERT(config->isValid == 0x0100);
    CPPUNIT_ASSERT(config->channel == RS9110_UART::CHANNEL_24_6);
    CPPUNIT_ASSERT(config->nwType == RS9110_UART::NW_TYPE_RSP_INFRA);
    CPPUNIT_ASSERT(config->secMode == RS9110_UART::SEC_MODE_RSP_WPA2);
    CPPUNIT_ASSERT(config->dataRate == RS9110_UART::TX_RATE_11);
    CPPUNIT_ASSERT(config->powerLevel == RS9110_UART::TX_POWER_HIGH);
    CPPUNIT_ASSERT(strcmp(config->psk, " Redpine_net_password") == 0);
    CPPUNIT_ASSERT(strcmp(config->ssid, " Redpine_net") == 0);
    CPPUNIT_ASSERT(config->dhcp == RS9110_UART::DHCP_DHCP);
    CPPUNIT_ASSERT(memcmp(config->address, "\x01\x02\x03\x04", sizeof(config->address)) == 0);
    CPPUNIT_ASSERT(memcmp(config->subnet,  "\x05\x06\x07\x08", sizeof(config->subnet)) == 0);
    CPPUNIT_ASSERT(memcmp(config->gateway, "\x09\x10\x11\x12", sizeof(config->gateway)) == 0);
    CPPUNIT_ASSERT(memcmp(config->featureSelect, "\xAA\xBB\xCC\xDD", sizeof(config->featureSelect)) == 0);
}


void RS9110_UART_Test::SendBandTest ()
{
    bool bRtn;


    bRtn = rs->Band(RS9110_UART::BAND_2_4_GHZ);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_BAND=0\r\n");
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_BAND);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->Band(RS9110_UART::BAND_5_GHZ);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_BAND=1\r\n");

    bRtn = rs->Band(RS9110_UART::BAND_MAX);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
}


void RS9110_UART_Test::SendInitTest ()
{
    bool bRtn;


    bRtn = rs->Init();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_INIT);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_INIT\r\n");
}


void RS9110_UART_Test::GetNumScanResultsTest ()
{
    bool bRtn;


    bRtn = rs->GetNumScanResults();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_SCAN_RESULTS);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_NUMSCAN?\r\n");
}


void RS9110_UART_Test::SetNumScanResultsTest ()
{
    bool bRtn;


    bRtn = rs->SetNumScanResults(0);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->SetNumScanResults(11);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->SetNumScanResults(10);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SET_SCAN_RESULTS);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_NUMSCAN=10\r\n");
}


void RS9110_UART_Test::PassiveScanTest ()
{
    bool bRtn;


    bRtn = rs->PassiveScan(9);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_PASSIVE_SCAN);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_PASSSCAN=9\r\n");
}


void RS9110_UART_Test::ScanTest ()
{
    bool bRtn;


    bRtn = rs->Scan(6);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SCAN);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_SCAN=6\r\n");

    bRtn = rs->Scan(12, "Redpine_net");
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_SCAN=12,Redpine_net\r\n");

    bRtn = rs->Scan(0, "01234567890123456789012345678901");
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_SCAN=0,01234567890123456789012345678901\r\n");

    bRtn = rs->Scan(0, "");
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_SCAN=0\r\n");

    bRtn = rs->Scan(0, "012345678901234567890123456789012");
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
}


void RS9110_UART_Test::NextScanTest ()
{
    bool bRtn;


    bRtn = rs->NextScan();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_NEXT_SCAN);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_NEXTSCAN\r\n");
}


void RS9110_UART_Test::GetMACOfAPsTest ()
{
    bool bRtn;


    bRtn = rs->GetMACOfAPs();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_MAC_APS);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_BSSID?\r\n");
}


void RS9110_UART_Test::GetNetworkTypeTest ()
{
    bool bRtn;


    bRtn = rs->GetNetworkType();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_NETWORK_TYPE);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_NWTYPE?\r\n");
}


void RS9110_UART_Test::SetNetworkTypeTest ()
{
    bool bRtn;


    bRtn = rs->SetNetworkType(RS9110_UART::NW_TYPE_INFRASTRUCTURE);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SET_NETWORK_TYPE);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_NETWORK=INFRASTRUCTURE\r\n");
    
    bRtn = rs->SetNetworkType(RS9110_UART::NW_TYPE_INFRASTRUCTURE, RS9110_UART::IBSS_TYPE_JOINER, 0);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_NETWORK=INFRASTRUCTURE\r\n");
    
    bRtn = rs->SetNetworkType(RS9110_UART::NW_TYPE_IBSS, RS9110_UART::IBSS_TYPE_JOINER, 11);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_NETWORK=IBSS,0,11\r\n");
    
    bRtn = rs->SetNetworkType(RS9110_UART::NW_TYPE_IBSS, RS9110_UART::IBSS_TYPE_CREATOR, 11);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_NETWORK=IBSS,1,11\r\n");
    
    bRtn = rs->SetNetworkType(RS9110_UART::NW_TYPE_IBSS_SEC, RS9110_UART::IBSS_TYPE_JOINER, 0);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_NETWORK=IBSS_SEC,0,0\r\n");
    
    bRtn = rs->SetNetworkType(RS9110_UART::NW_TYPE_IBSS_SEC, RS9110_UART::IBSS_TYPE_CREATOR, 0);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_NETWORK=IBSS_SEC,1,0\r\n");
}


void RS9110_UART_Test::PSKTest ()
{
    bool bRtn;


    bRtn = rs->PSK("a");
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_PSK);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_PSK=a\r\n");

    bRtn = rs->PSK("012345678901234567890123456789012345678901234567890123456789012");
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_PSK=012345678901234567890123456789012345678901234567890123456789012\r\n");

    bRtn = rs->PSK("0123456789012345678901234567890123456789012345678901234567890123");
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->PSK("");
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->PSK(NULL);
    CPPUNIT_ASSERT(bRtn == false);
}


void RS9110_UART_Test::SetWEPKeysTest ()
{
    bool bRtn;


    bRtn = rs->SetWEPKeys(2, "ABCDE12345", "ABCDE12346", "ABCDE12347");
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_WEP_KEYS);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_WEP_KEYS=2,ABCDE12345,ABCDE12346,ABCDE12347\r\n");

    bRtn = rs->SetWEPKeys(2, NULL, "ABCDE12346", "ABCDE12347");
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->SetWEPKeys(2, "ABCDE12345", NULL, "ABCDE12347");
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->SetWEPKeys(2, "ABCDE12345", "ABCDE12346", NULL);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->SetWEPKeys(4, "ABCDE12345", "ABCDE12346", "ABCDE12347");
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
}


void RS9110_UART_Test::AuthModeTest ()
{
    bool bRtn;


    bRtn = rs->AuthMode(RS9110_UART::AUTH_MODE_WEP_OPEN);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_AUTH_MODE);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_AUTHMODE=0\r\n");

    bRtn = rs->AuthMode(RS9110_UART::AUTH_MODE_WEP_SHARED);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_AUTHMODE=1\r\n");

    bRtn = rs->AuthMode(RS9110_UART::AUTH_MODE_WPA);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_AUTHMODE=2\r\n");

    bRtn = rs->AuthMode(RS9110_UART::AUTH_MODE_WPA2);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_AUTHMODE=3\r\n");

    bRtn = rs->AuthMode(RS9110_UART::AUTH_MODE_OPEN);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_AUTHMODE=4\r\n");
}


void RS9110_UART_Test::JoinTest ()
{
    bool bRtn;


    bRtn = rs->Join("Redpine", RS9110_UART::TX_RATE_54, RS9110_UART::TX_POWER_MEDIUM);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_JOIN);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_JOIN=Redpine,12,1\r\n");

    bRtn = rs->Join("01234567890123456789012345678901", RS9110_UART::TX_RATE_AUTO, RS9110_UART::TX_POWER_HIGH);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_JOIN=01234567890123456789012345678901,0,2\r\n");

    bRtn = rs->Join("012345678901234567890123456789012", RS9110_UART::TX_RATE_48, RS9110_UART::TX_POWER_LOW);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->Join("", RS9110_UART::TX_RATE_54, RS9110_UART::TX_POWER_MEDIUM);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->Join(NULL, RS9110_UART::TX_RATE_54, RS9110_UART::TX_POWER_MEDIUM);
    CPPUNIT_ASSERT(bRtn == false);
}


void RS9110_UART_Test::DisassociateTest ()
{
    bool bRtn;


    bRtn = rs->Disassociate();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_DISASSOCIATE);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_DISASSOC\r\n");
}


void RS9110_UART_Test::PowerModeTest ()
{
	bool bRtn;


	bRtn = rs->PowerMode(RS9110_UART::PW_MODE_0);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_POWER_MODE);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_PWMODE=0\r\n");

	bRtn = rs->PowerMode(RS9110_UART::PW_MODE_1);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_POWER_MODE);
    CompareStream("AT+RSI_PWMODE=1\r\n");

	bRtn = rs->PowerMode(RS9110_UART::PW_MODE_2);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_POWER_MODE);
    CompareStream("AT+RSI_PWMODE=2\r\n");
}


void RS9110_UART_Test::KeepSleepingTest ()
{
	bool bRtn;


	bRtn = rs->KeepSleeping();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_KEEP_SLEEPING);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("ACK\r\n");
}


void RS9110_UART_Test::SetSleepTimerTest ()
{
	bool bRtn;


	bRtn = rs->SetSleepTimer(0);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

	bRtn = rs->SetSleepTimer(10001);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

	bRtn = rs->SetSleepTimer(168);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SLEEP_TIMER);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_SLEEPTIMER=168\r\n");
}


void RS9110_UART_Test::SetFeatureSelectTest ()
{
    bool bRtn;

    RS9110_UART::TFeatureSelect featSel;
    featSel.value = 0;
    featSel.authModeRelevance = 1;

    bRtn = rs->SetFeatureSelect(featSel);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_FEATURE_SELECT);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_FEAT_SEL=2048\r\n");
}


void RS9110_UART_Test::IPConfigurationTest ()
{
    bool bRtn;


    bRtn = rs->IPConfiguration(RS9110_UART::DHCP_MANUAL, "", "", "");
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->IPConfiguration(RS9110_UART::DHCP_MANUAL, NULL, NULL, NULL);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->IPConfiguration(RS9110_UART::DHCP_MANUAL, "000.000.000.000", "111.111.111.111", "222.222.222.222");
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_IP_CONF);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_IPCONF=0,000.000.000.000,111.111.111.111,222.222.222.222\r\n");

    bRtn = rs->IPConfiguration(RS9110_UART::DHCP_DHCP, "", "", "");
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_IPCONF=1,0,0\r\n");

    bRtn = rs->IPConfiguration(RS9110_UART::DHCP_MAX, "", "", "");
    CPPUNIT_ASSERT(bRtn == false);
}


void RS9110_UART_Test::OpenTcpSocketTest ()
{
    bool bRtn;


    bRtn = rs->OpenTcpSocket("", 5000, 5001);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->OpenTcpSocket(NULL, 5000, 5001);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->OpenTcpSocket("000.000.000.000", 5000, 1023);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->OpenTcpSocket("000.000.000.000", 5000, 49152);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->OpenTcpSocket("000.000.000.000", 5000, 5001);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_OPEN_TCP_SOCKET);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_TCP=000.000.000.000,5000,5001\r\n");
}


void RS9110_UART_Test::OpenListeningUdpSocketTest ()
{
    bool bRtn;


    bRtn = rs->OpenListeningUdpSocket(0);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_OPEN_LUDP_SOCKET);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_LUDP=0\r\n");

    bRtn = rs->OpenListeningUdpSocket(65535);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_LUDP=65535\r\n");
}


void RS9110_UART_Test::OpenUdpSocketTest ()
{
    bool bRtn;


    bRtn = rs->OpenUdpSocket("", 5000, 5001);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->OpenUdpSocket(NULL, 5000, 5001);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->OpenUdpSocket("000.000.000.000", 8000, 1234);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_OPEN_UDP_SOCKET);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_UDP=000.000.000.000,8000,1234\r\n");
}


void RS9110_UART_Test::OpenListeningTcpSocketTest ()
{
    bool bRtn;


    bRtn = rs->OpenListeningTcpSocket(0);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_OPEN_LTCP_SOCKET);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_LTCP=0\r\n");

    bRtn = rs->OpenListeningTcpSocket(65535);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_LTCP=65535\r\n");
}


void RS9110_UART_Test::GetSocketStatusTest ()
{
    bool bRtn;


    bRtn = rs->GetSocketStatus(0);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->GetSocketStatus(11);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->GetSocketStatus(1);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_SOCKET_STATUS);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_CTCP=1\r\n");

    bRtn = rs->GetSocketStatus(7);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_CTCP=7\r\n");
}


void RS9110_UART_Test::CloseSocketTest ()
{
    bool bRtn;


    bRtn = rs->CloseSocket(0);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->CloseSocket(11);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->CloseSocket(1);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_CLOSE_SOCKET);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_CLS=1\r\n");

    bRtn = rs->CloseSocket(7);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_CLS=7\r\n");
}


void RS9110_UART_Test::SendTest ()
{
    unsigned int iRtn;
    unsigned int BUFFER_SIZE = 2000;
    char *data = new char[BUFFER_SIZE];
    char *result = new char[BUFFER_SIZE];
    int hdrLen = 0;


    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        data[i]     = 0;
        result[i]   = 0;
    }

    iRtn = rs->Send(1, RS9110_UART::SOCKET_UDP, "123.123.123.123", 12345, "A", 1);
    CPPUNIT_ASSERT(iRtn == 1);
    CompareStream("AT+RSI_SND=1,0,123.123.123.123,12345,A\r\n");

    data[0] = (char) 0x0D; data[1] = (char) 0x0A;
    data[2] = (char) 0xDB;
    data[3] = (char) 0x0D; data[4] = (char) 0x0A;
    data[5] = 'H'; data[6] = 'o'; data[7] = 'l'; data[8] = 'a'; data[9] = '\0';
    data[10] = (char) 0xDB;
    iRtn = rs->Send(1, RS9110_UART::SOCKET_UDP, "123.123.123.123", 12345, data, 11);
    CPPUNIT_ASSERT(iRtn == 11);
    strcpy(result, "AT+RSI_SND=1,0,123.123.123.123,12345,");
    hdrLen = strlen(result);
    data[0] = (char) 0xDB; data[1] = (char) 0xDC;
    data[2] = (char) 0xDB; data[3] = (char) 0xDD;
    data[4] = (char) 0xDB; data[5] = (char) 0xDC;
    data[6] = 'H'; data[7] = 'o'; data[8] = 'l'; data[9] = 'a'; data[10] = '\0';
    data[11] = (char) 0xDB; data[12] = (char) 0xDD;
    data[13] = (char) 0x0D; data[14] = (char) 0x0A;
    memcpy(&result[hdrLen], data, 15);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SEND_DATA);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream(result, (hdrLen + 15));
    
    for(int i = 0; i < RS9110_UART::MAX_SEND_DATA_SIZE_UDP; i++) data[i] = (char) 0xDB;
    iRtn = rs->Send(1, RS9110_UART::SOCKET_UDP, "123.123.123.123", 12345, data, RS9110_UART::MAX_SEND_DATA_SIZE_UDP);
    CPPUNIT_ASSERT(iRtn == (RS9110_UART::MAX_SEND_DATA_SIZE_UDP / 2));
    strcpy(result, "AT+RSI_SND=1,0,123.123.123.123,12345,");
    hdrLen = strlen(result);
    int i = 0;
    while(i < RS9110_UART::MAX_SEND_DATA_SIZE_UDP)
    {
        data[i++] = (char) 0xDB;
        data[i++] = (char) 0xDD;
    }

    data[RS9110_UART::MAX_SEND_DATA_SIZE_UDP] = (char) 0x0D; data[RS9110_UART::MAX_SEND_DATA_SIZE_UDP + 1] = (char) 0x0A;
    memcpy(&result[hdrLen], data, RS9110_UART::MAX_SEND_DATA_SIZE_UDP + 2);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SEND_DATA);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream(result, (hdrLen + RS9110_UART::MAX_SEND_DATA_SIZE_UDP + 2));
    
    delete data;
    delete result;
}


void RS9110_UART_Test::GetDNSTest ()
{
    bool bRtn;


    bRtn = rs->GetDNS("");
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);

    bRtn = rs->GetDNS(NULL);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->GetDNS("www.google.com");
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_DNS);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_DNSGET=www.google.com\r\n");
}


void RS9110_UART_Test::GetFirmwareVersionTest ()
{
    bool bRtn;


    bRtn = rs->GetFirmwareVersion();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_FW_VERSION);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_FWVERSION?\r\n");
}


void RS9110_UART_Test::GetNetworkParametersTest ()
{
    bool bRtn;


    bRtn = rs->GetNetworkParameters();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_NETWORK_PARAMS);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_NWPARAMS?\r\n");
}


void RS9110_UART_Test::ResetTest ()
{
    bool bRtn;


    bRtn = rs->Reset();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_RESET);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_RESET\r\n");
}


void RS9110_UART_Test::GetMACAddressTest ()
{
    bool bRtn;


    bRtn = rs->GetMACAddress();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_MAC);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_MAC?\r\n");
}


void RS9110_UART_Test::GetRSSITest ()
{
    bool bRtn;


    bRtn = rs->GetRSSI();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_RSSI);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_RSSI?\r\n");
}


void RS9110_UART_Test::SaveConfigurationTest ()
{
    bool bRtn;


    bRtn = rs->SaveConfiguration();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SAVE_CONFIG);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_CFGSAVE\r\n");
}


void RS9110_UART_Test::ConfigurationTest ()
{
    bool bRtn;


    bRtn = rs->Configuration(RS9110_UART::CONFIG_ENABLE);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_ENABLE_CONFIG);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_CFGENABLE=1\r\n");

    bRtn = rs->Configuration(RS9110_UART::CONFIG_DISABLE);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_CFGENABLE=0\r\n");

    bRtn = rs->Configuration(RS9110_UART::CONFIG_MAX);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
}


void RS9110_UART_Test::GetConfigurationTest ()
{
    bool bRtn;


    bRtn = rs->GetConfiguration();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_CONFIG);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_MAX);
    CompareStream("AT+RSI_CFGGET?\r\n");
}
