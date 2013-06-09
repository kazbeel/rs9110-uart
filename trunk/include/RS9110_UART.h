#ifndef _RS9110_UART_H_
#define _RS9110_UART_H_

#include "IPersistor.h"

#if defined (WIN32)
#include <stddef.h>
#endif /* WIN32 */


class RS9110_UART
{
private:

    /* CONSTANTS */
    static const unsigned char  MAX_SSID_LEN			= 32;
    static const unsigned char  MAX_PSK_LEN				= 63;
    static const unsigned short MAX_SEND_DATA_SIZE_UDP	= 1472;
    static const unsigned short MAX_SEND_DATA_SIZE_TCP  = 1460;
	static const unsigned char	MIN_SOCKET_HANDLE		= 1;
	static const unsigned char	MAX_SOCKET_HANDLE		= 7;
	static const unsigned short	MIN_TCP_SOCKET_PORT		= 1024;
	static const unsigned short MAX_TCP_SOCKET_PORT		= 49151;
	static const unsigned int	MAX_BUFFER_SIZE			= 1515;
    static const unsigned char  MAX_NUM_SCAN_RESULTS    = 10;

public:

    /* ENUMS */
    enum ECommand
    {
        CMD_BAND = 0,
        CMD_INIT,
        CMD_SCAN,
        CMD_SET_NETWORK_TYPE,
        CMD_PSK,
        CMD_AUTH_MODE,
        CMD_JOIN,
		CMD_RESET,
		CMD_IP_CONF,
		CMD_CLOSE_SOCKET,
		CMD_OPEN_TCP_SOCKET,
		CMD_SAVE_CONFIG,
		CMD_ENABLE_CONFIG,
		CMD_GET_CONFIG,
		CMD_DISASSOCIATE,
		CMD_FW_VERSION,
		CMD_SEND_DATA,
        CMD_GET_MAC,
        CMD_GET_RSSI,
        CMD_GET_NETWORK_PARAMS,
        CMD_OPEN_UDP_SOCKET,
        CMD_OPEN_LISTENING_TCP_SOCKET,
        CMD_OPEN_LISTENING_UDP_SOCKET,
        CMD_GET_SOCKET_STATUS,
        CMD_GET_SCAN_RESULTS,
        CMD_SET_SCAN_RESULTS,
        CMD_NEXT_SCAN,
        CMD_GET_MAC_APS,
        CMD_GET_NETWORK_TYPE,
        CMD_GET_DNS,
        CMD_MAX
    };

    enum EResponseType
    {
        RESP_TYPE_OK = 0,
        RESP_TYPE_ERROR,
        RESP_TYPE_READ,
        RESP_TYPE_CLOSE,
        RESP_TYPE_UNKNOWN,
        RESP_TYPE_MAX
    };

    enum EErrorCode
    {
        ERROR_NONE = 0,
        ERROR_NO_EXIST_TCP_SERVER = 0x87,
        ERROR_MAX
    };

    enum EBand
    {
        BAND_2_4_GHZ = 0,
        BAND_5_GHZ,
        BAND_MAX
    };

    enum ESecurityMode
    {
        SEC_MODE_OPEN = 0,
        SEC_MODE_WPA,
        SEC_MODE_WPA2,
        SEC_MODE_WEP,
        SEC_MODE_MAX
    };

    enum ENetworkType
    {
        NW_TYPE_INFRASTRUCTURE = 0,
        NW_TYPE_IBSS,
        NW_TYPE_IBSS_SEC,
        NW_TYPE_MAX
    };

    enum EIBSSType
    {
        IBSS_TYPE_JOINER = 0,
        IBSS_TYPE_CREATOR,
        IBSS_TYPE_MAX
    };

    enum EAuthMode
    {
        AUTH_MODE_WEP_OPEN = 0,
        AUTH_MODE_WEP_SHARED,
        AUTH_MODE_WPA,
        AUTH_MODE_WPA2,
        AUTH_MODE_OPEN,
        AUTH_MODE_MAX
    };

    enum ETxRate
    {
        TX_RATE_AUTO = 0,
        TX_RATE_1,
        TX_RATE_2,
        TX_RATE_5_5,
        TX_RATE_11,
        TX_RATE_6,
        TX_RATE_9,
        TX_RATE_12,
        TX_RATE_18,
        TX_RATE_24,
        TX_RATE_36,
        TX_RATE_48,
        TX_RATE_54,
        TX_RATE_MCS0,
        TX_RATE_MCS1,
        TX_RATE_MCS2,
        TX_RATE_MCS3,
        TX_RATE_MCS4,
        TX_RATE_MCS5,
        TX_RATE_MCS6,
        TX_RATE_MCS7,
        TX_RATE_MAX
    };

    enum ETxPower
    {
        TX_POWER_LOW = 0,
        TX_POWER_MEDIUM,
        TX_POWER_HIGH,
        TX_POWER_MAX
    };

	enum EDHCPMode
	{
		DHCP_MANUAL = 0,
		DHCP_DHCP,
		DHCP_MAX
	};

    enum EConfiguration
    {
        CONFIG_DISABLE = 0,
        CONFIG_ENABLE,
        CONFIG_MAX
    };


    /* STRUCTURES */
    struct TScanResponse
    {
        char            ssid[MAX_SSID_LEN];
        unsigned char   mode;
        unsigned char   rssi;
    };


    /* METHODS */
    RS9110_UART (IPersistor *persistor);
    ~RS9110_UART ();

    void            SetPersistor (IPersistor *persistor);

    bool            ProcessMessage (char *message, unsigned int size);

    ECommand        GetLastCommand ();
    EResponseType   GetResponseType ();
    EErrorCode      GetErrorCode ();

    bool            Band                    (EBand eBand);
    bool            Init                    ();
    bool            GetNumScanResults       ();
    bool            SetNumScanResults       (unsigned char value);
    /*! @todo Passive Scan */
    bool            Scan                    (unsigned char channel, const char *ssid = NULL);
    bool            NextScan                ();
    bool            GetMACOfAPs             ();
    bool            GetNetworkType          ();
    bool            SetNetworkType          (ENetworkType eNWType, EIBSSType eIBSSType = IBSS_TYPE_MAX, unsigned char channel = 0);
    bool            PSK                     (const char *psk);
    /*! @todo Set WEP Key */
    bool            AuthMode                (EAuthMode eAuthMode);
    bool            Join                    (const char *ssid, ETxRate eTxRate, ETxPower eTxPower);
	bool            Disassociate            ();
    
    /*! @todo Power Modes */

    bool            IPConfiguration         (EDHCPMode eDHCPMode, const char *ipAddr, const char *subNetwork, const char *gateway);
	bool            OpenTcpSocket           (const char *hostIpAddr, unsigned short targetPort, unsigned short localPort);
    bool            OpenListeningUdpSocket  (unsigned short localPort);
    bool            OpenUdpSocket           (const char *hostIpAddr, unsigned short targetPort, unsigned short localPort);
    bool            OpenListeningTcpSocket  (unsigned short localPort);
    bool            GetSocketStatus         (unsigned char socketId);
	bool            CloseSocket             (unsigned char socketId);
    unsigned int    Send                    (unsigned char socketId, const char *hostIpAddr, unsigned short hostPort, const char *data, unsigned int dataSize);
    /*! @todo Receive Data on a Socket */
    bool            GetDNS                  (const char *domainName);

	bool            GetFirmwareVersion      ();
    bool            GetNetworkParameters    ();
    bool            Reset                   ();
    bool            GetMACAddress           ();
    bool            GetRSSI                 ();

	bool            SaveConfiguration       ();
	bool            Configuration           (EConfiguration eConfig);
	bool            GetConfiguration        ();


private:

    /* METHODS */
    void ProcessResponseType (const char *message);
    bool IsValidSocketId (unsigned char socketId);


    /* VARIABLES */
    IPersistor     *_persistor;
    char            _buffer[MAX_BUFFER_SIZE];
    ECommand        _lastCommand;
    EResponseType   _responseType;
    EErrorCode      _errorCode;
};

#endif /* _RS9110_UART_H_ */