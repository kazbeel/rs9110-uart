#pragma once

#include "PersistorWin32Mock.h"
#include "RS9110_UART.h"

#include <cppunit\extensions\HelperMacros.h>


class RS9110_UART_Test : public CPPUNIT_NS::TestFixture
{
CPPUNIT_TEST_SUITE(RS9110_UART_Test);
    //CPPUNIT_TEST(ProcessMessageTest);

    CPPUNIT_TEST(SendBandTest);
    CPPUNIT_TEST(SendInitTest);
    CPPUNIT_TEST(GetNumScanResultsTest);
    CPPUNIT_TEST(SetNumScanResultsTest);
    /*! @todo Passive Scan Test */
    CPPUNIT_TEST(ScanTest);
    CPPUNIT_TEST(NextScanTest);
    CPPUNIT_TEST(GetMACOfAPsTest);
    CPPUNIT_TEST(GetNetworkTypeTest);
    CPPUNIT_TEST(SetNetworkTypeTest);
    CPPUNIT_TEST(PSKTest);
    /*! @todo Set WEP keys */
    CPPUNIT_TEST(AuthModeTest);
    CPPUNIT_TEST(JoinTest);
    CPPUNIT_TEST(DisassociateTest);

	CPPUNIT_TEST(PowerModeTest);
	CPPUNIT_TEST(KeepSleepingTest);
	CPPUNIT_TEST(SetSleepTimerTest);

  	CPPUNIT_TEST(SetFeatureSelectTest);

    CPPUNIT_TEST(IPConfigurationTest);
    CPPUNIT_TEST(OpenTcpSocketTest);
    CPPUNIT_TEST(OpenListeningUdpSocketTest);
    CPPUNIT_TEST(OpenUdpSocketTest);
    CPPUNIT_TEST(OpenListeningTcpSocketTest);
    CPPUNIT_TEST(GetSocketStatusTest);
    CPPUNIT_TEST(CloseSocketTest);
    CPPUNIT_TEST(SendTest);
    CPPUNIT_TEST(GetDNSTest);

    CPPUNIT_TEST(GetFirmwareVersionTest);
    CPPUNIT_TEST(GetNetworkParametersTest);
    CPPUNIT_TEST(ResetTest);
    CPPUNIT_TEST(GetMACAddressTest);
    CPPUNIT_TEST(GetRSSITest);

    CPPUNIT_TEST(SaveConfigurationTest);
    CPPUNIT_TEST(ConfigurationTest);
    CPPUNIT_TEST(GetConfigurationTest);
CPPUNIT_TEST_SUITE_END();


public:

    void setUp ();
    void tearDown ();

    void CompareStream (const char *stream, unsigned int size = 0);

    void ProcessMessageTest ();

    void SendBandTest ();
    void SendInitTest ();
    void GetNumScanResultsTest ();
    void SetNumScanResultsTest ();
    /*! @todo Passive Scan Test */
    void ScanTest ();
    void NextScanTest ();
    void GetMACOfAPsTest ();
    void GetNetworkTypeTest ();
    void SetNetworkTypeTest ();
    void PSKTest ();
    /*! @todo Set WEP keys */
    void AuthModeTest ();
    void JoinTest ();
    void DisassociateTest ();

    void PowerModeTest ();
	void KeepSleepingTest ();
	void SetSleepTimerTest ();

    void SetFeatureSelectTest ();

    void IPConfigurationTest ();
    void OpenTcpSocketTest ();
    void OpenListeningUdpSocketTest ();
    void OpenUdpSocketTest ();
    void OpenListeningTcpSocketTest ();
    void GetSocketStatusTest ();
    void CloseSocketTest ();
    void SendTest ();
    void GetDNSTest ();

    void GetFirmwareVersionTest ();
    void GetNetworkParametersTest ();
    void ResetTest ();
    void GetMACAddressTest ();
    void GetRSSITest ();

    void SaveConfigurationTest ();
    void ConfigurationTest ();
    void GetConfigurationTest ();


protected:
    
    PersistorWin32Mock     *mockFile;
    RS9110_UART            *rs;

};
