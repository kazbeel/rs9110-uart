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


    bRtn = rs->ProcessMessage("OK\r\n");
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_OK);
    CPPUNIT_ASSERT(rs->GetErrorCode() == RS9110_UART::ERROR_NONE);

    bRtn = rs->ProcessMessage("ERROR‡\r\n");
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetResponseType() == RS9110_UART::RESP_TYPE_ERROR);
    CPPUNIT_ASSERT(rs->GetErrorCode() == RS9110_UART::ERROR_NO_EXIST_TCP_SERVER);

    //! @todo RESP_TYPE_READ

    bRtn = rs->ProcessMessage("UNKNOWN\r\n");
    CPPUNIT_ASSERT(bRtn == false);
}


void RS9110_UART_Test::SendBandTest ()
{
    bool bRtn;


    bRtn = rs->Band(RS9110_UART::BAND_2_4_GHZ);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_BAND=0\r\n");
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_BAND);

    bRtn = rs->Band(RS9110_UART::BAND_5_GHZ);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_BAND=1\r\n");

    bRtn = rs->Band(RS9110_UART::BAND_MAX);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
}


void RS9110_UART_Test::SendInitTest ()
{
    bool bRtn;


    bRtn = rs->Init();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_INIT);
    CompareStream("AT+RSI_INIT\r\n");
}


void RS9110_UART_Test::GetNumScanResultsTest ()
{
    bool bRtn;


    bRtn = rs->GetNumScanResults();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_SCAN_RESULTS);
    CompareStream("AT+RSI_NUMSCAN?\r\n");
}


void RS9110_UART_Test::SetNumScanResultsTest ()
{
    bool bRtn;


    bRtn = rs->SetNumScanResults(0);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);

    bRtn = rs->SetNumScanResults(11);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->SetNumScanResults(10);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SET_SCAN_RESULTS);
    CompareStream("AT+RSI_NUMSCAN=10\r\n");
}


void RS9110_UART_Test::ScanTest ()
{
    bool bRtn;


    bRtn = rs->Scan(6);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SCAN);
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
}


void RS9110_UART_Test::NextScanTest ()
{
    bool bRtn;


    bRtn = rs->NextScan();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_NEXT_SCAN);
    CompareStream("AT+RSI_NEXTSCAN\r\n");
}


void RS9110_UART_Test::GetMACOfAPsTest ()
{
    bool bRtn;


    bRtn = rs->GetMACOfAPs();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_MAC_APS);
    CompareStream("AT+RSI_BSSID?\r\n");
}


void RS9110_UART_Test::GetNetworkTypeTest ()
{
    bool bRtn;


    bRtn = rs->GetNetworkType();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_NETWORK_TYPE);
    CompareStream("AT+RSI_NWTYPE?\r\n");
}


void RS9110_UART_Test::SetNetworkTypeTest ()
{
    bool bRtn;


    bRtn = rs->SetNetworkType(RS9110_UART::NW_TYPE_INFRASTRUCTURE);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SET_NETWORK_TYPE);
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
    CompareStream("AT+RSI_PSK=a\r\n");

    bRtn = rs->PSK("012345678901234567890123456789012345678901234567890123456789012");
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_PSK=012345678901234567890123456789012345678901234567890123456789012\r\n");

    bRtn = rs->PSK("0123456789012345678901234567890123456789012345678901234567890123");
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);

    bRtn = rs->PSK("");
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->PSK(NULL);
    CPPUNIT_ASSERT(bRtn == false);
}


void RS9110_UART_Test::AuthModeTest ()
{
    bool bRtn;


    bRtn = rs->AuthMode(RS9110_UART::AUTH_MODE_WEP_OPEN);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_AUTH_MODE);
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
    CompareStream("AT+RSI_JOIN=Redpine,12,1\r\n");

    bRtn = rs->Join("01234567890123456789012345678901", RS9110_UART::TX_RATE_AUTO, RS9110_UART::TX_POWER_HIGH);
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_JOIN=01234567890123456789012345678901,0,2\r\n");

    bRtn = rs->Join("012345678901234567890123456789012", RS9110_UART::TX_RATE_48, RS9110_UART::TX_POWER_LOW);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);

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
    CompareStream("AT+RSI_DISASSOC\r\n");
}


void RS9110_UART_Test::PowerModeTest ()
{
	bool bRtn;


	bRtn = rs->PowerMode(RS9110_UART::PW_MODE_0);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_POWER_MODE);
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
    CompareStream("ACK\r\n");
}


void RS9110_UART_Test::SetSleepTimerTest ()
{
	bool bRtn;


	bRtn = rs->SetSleepTimer(0);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);

	bRtn = rs->SetSleepTimer(10001);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);

	bRtn = rs->SetSleepTimer(168);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SLEEP_TIMER);
    CompareStream("AT+RSI_SLEEPTIMER=168\r\n");
}


void RS9110_UART_Test::SetFeatureSelectTest ()
{
    bool bRtn;


    bRtn = rs->SetFeatureSelect(1024);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_FEATURE_SELECT);
    CompareStream("AT+RSI_FEAT_SEL=1024\r\n");
}


void RS9110_UART_Test::IPConfigurationTest ()
{
    bool bRtn;


    bRtn = rs->IPConfiguration(RS9110_UART::DHCP_MANUAL, "", "", "");
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);

    bRtn = rs->IPConfiguration(RS9110_UART::DHCP_MANUAL, NULL, NULL, NULL);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->IPConfiguration(RS9110_UART::DHCP_MANUAL, "000.000.000.000", "111.111.111.111", "222.222.222.222");
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_IP_CONF);
    CompareStream("AT+RSI_IPCONF=0,000.000.000.000,111.111.111.111,222.222.222.222\r\n");

    bRtn = rs->IPConfiguration(RS9110_UART::DHCP_DHCP, "", "", "");
    CPPUNIT_ASSERT(bRtn == true);
    CompareStream("AT+RSI_IPCONF=1,0,0,0\r\n");

    bRtn = rs->IPConfiguration(RS9110_UART::DHCP_MAX, "", "", "");
    CPPUNIT_ASSERT(bRtn == false);
}


void RS9110_UART_Test::OpenTcpSocketTest ()
{
    bool bRtn;


    bRtn = rs->OpenTcpSocket("", 5000, 5001);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);

    bRtn = rs->OpenTcpSocket(NULL, 5000, 5001);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);

    bRtn = rs->OpenTcpSocket("000.000.000.000", 5000, 1023);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->OpenTcpSocket("000.000.000.000", 5000, 49152);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->OpenTcpSocket("000.000.000.000", 5000, 5001);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_OPEN_TCP_SOCKET);
    CompareStream("AT+RSI_TCP=000.000.000.000,5000,5001\r\n");
}


void RS9110_UART_Test::OpenListeningUdpSocketTest ()
{
    bool bRtn;


    bRtn = rs->OpenListeningUdpSocket(0);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_OPEN_LUDP_SOCKET);
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

    bRtn = rs->OpenUdpSocket(NULL, 5000, 5001);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->OpenUdpSocket("000.000.000.000", 8000, 1234);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_OPEN_UDP_SOCKET);
    CompareStream("AT+RSI_UDP=000.000.000.000,8000,1234\r\n");
}


void RS9110_UART_Test::OpenListeningTcpSocketTest ()
{
    bool bRtn;


    bRtn = rs->OpenListeningTcpSocket(0);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_OPEN_LTCP_SOCKET);
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

    bRtn = rs->GetSocketStatus(11);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->GetSocketStatus(1);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_SOCKET_STATUS);
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

    bRtn = rs->CloseSocket(11);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->CloseSocket(1);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_CLOSE_SOCKET);
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

    bRtn = rs->GetDNS(NULL);
    CPPUNIT_ASSERT(bRtn == false);

    bRtn = rs->GetDNS("www.google.com");
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_DNS);
    CompareStream("AT+RSI_DNSGET=www.google.com\r\n");
}


void RS9110_UART_Test::GetFirmwareVersionTest ()
{
    bool bRtn;


    bRtn = rs->GetFirmwareVersion();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_FW_VERSION);
    CompareStream("AT+RSI_FWVERSION?\r\n");
}


void RS9110_UART_Test::GetNetworkParametersTest ()
{
    bool bRtn;


    bRtn = rs->GetNetworkParameters();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_NETWORK_PARAMS);
    CompareStream("AT+RSI_NWPARAMS?\r\n");
}


void RS9110_UART_Test::ResetTest ()
{
    bool bRtn;


    bRtn = rs->Reset();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_RESET);
    CompareStream("AT+RSI_RESET\r\n");
}


void RS9110_UART_Test::GetMACAddressTest ()
{
    bool bRtn;


    bRtn = rs->GetMACAddress();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_MAC);
    CompareStream("AT+RSI_MAC?\r\n");
}


void RS9110_UART_Test::GetRSSITest ()
{
    bool bRtn;


    bRtn = rs->GetRSSI();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_RSSI);
    CompareStream("AT+RSI_RSSI?\r\n");
}


void RS9110_UART_Test::SaveConfigurationTest ()
{
    bool bRtn;


    bRtn = rs->SaveConfiguration();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_SAVE_CONFIG);
    CompareStream("AT+RSI_CFGSAVE\r\n");
}


void RS9110_UART_Test::ConfigurationTest ()
{
    bool bRtn;


    bRtn = rs->Configuration(RS9110_UART::CONFIG_ENABLE);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_ENABLE_CONFIG);
    CompareStream("AT+RSI_CFGENABLE=1\r\n");

    bRtn = rs->Configuration(RS9110_UART::CONFIG_DISABLE);
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_ENABLE_CONFIG);
    CompareStream("AT+RSI_CFGENABLE=0\r\n");

    bRtn = rs->Configuration(RS9110_UART::CONFIG_MAX);
    CPPUNIT_ASSERT(bRtn == false);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_MAX);
}


void RS9110_UART_Test::GetConfigurationTest ()
{
    bool bRtn;


    bRtn = rs->GetConfiguration();
    CPPUNIT_ASSERT(bRtn == true);
    CPPUNIT_ASSERT(rs->GetLastCommand() == RS9110_UART::CMD_GET_CONFIG);
    CompareStream("AT+RSI_CFGGET?\r\n");
}
