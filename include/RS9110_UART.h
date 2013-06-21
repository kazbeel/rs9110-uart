#ifndef _RS9110_UART_H_
#define _RS9110_UART_H_

#include "IPersistor.h"

#if defined (WIN32)
#include <stddef.h>
#endif /* WIN32 */


class RS9110_UART
{
public:

    /* CONSTANTS */
    static const unsigned char  MAX_SSID_LEN			= 32;
    static const unsigned char  MAX_PSK_LEN				= 63;
    static const unsigned short MAX_SEND_DATA_SIZE_UDP	= 1472;
    static const unsigned short MAX_SEND_DATA_SIZE_TCP  = 1460;
	static const unsigned char	MIN_SOCKET_HANDLE		= 1;
	static const unsigned char	MAX_SOCKET_HANDLE		= 7;
	static const unsigned short	MIN_TCP_SOCKET_PORT		= 1024;
	static const unsigned short MAX_TCP_SOCKET_PORT		= 49151;
	static const unsigned int	MAX_BUFFER_SIZE			= 1520;
    static const unsigned char  MAX_NUM_SCAN_RESULTS    = 10;
    static const unsigned char  MAC_ADDRESS_LEN         = 6;
    static const unsigned char  NW_ADDRESS_LEN          = 4;
    static const unsigned int   MAX_SLEEP_TIME_MS		= 10000;

    /* ENUMS */
    enum ECommand
    {
        CMD_BAND = 0,
        CMD_INIT,
        CMD_GET_SCAN_RESULTS,
        CMD_SET_SCAN_RESULTS,
        CMD_SCAN,
        CMD_NEXT_SCAN,
        CMD_GET_MAC_APS,
        CMD_GET_NETWORK_TYPE,
        CMD_SET_NETWORK_TYPE,
        CMD_PSK,
        CMD_AUTH_MODE,
        CMD_JOIN,
		CMD_DISASSOCIATE,
		CMD_POWER_MODE,
		CMD_KEEP_SLEEPING,
		CMD_SLEEP_TIMER,
		CMD_FEATURE_SELECT,
		CMD_IP_CONF,
		CMD_OPEN_TCP_SOCKET,
        CMD_OPEN_LUDP_SOCKET,
        CMD_OPEN_UDP_SOCKET,
        CMD_OPEN_LTCP_SOCKET,
        CMD_GET_SOCKET_STATUS,
		CMD_CLOSE_SOCKET,
		CMD_SEND_DATA,
        CMD_GET_DNS,
		CMD_FW_VERSION,
        CMD_GET_NETWORK_PARAMS,
        CMD_RESET,
        CMD_GET_MAC,
        CMD_GET_RSSI,
		CMD_SAVE_CONFIG,
		CMD_ENABLE_CONFIG,
		CMD_GET_CONFIG,
        CMD_MAX
    };

    enum EResponseType
    {
        RESP_TYPE_OK = 0,
        RESP_TYPE_ERROR,
        RESP_TYPE_READ,
        RESP_TYPE_CLOSE,
		RESP_TYPE_SLEEP,
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

    enum ESocketType
    {
        SOCKET_TCP = 0,
        SOCKET_UDP,
        SOCKET_LTCP,
        SOCKET_MULTICAST,
        SOCKET_LUDP,
        SOCKET_MAX
    };

    enum ENetworkTypeResp
    {
        NW_TYPE_RSP_INFRA = 0,
        NW_TYPE_RSP_ADHOC,
        NW_TYPE_RSP_MAX
    };

	enum EPowerMode
	{
		PW_MODE_0 = 0,
		PW_MODE_1,
		PW_MODE_2,
		PW_MODE_MAX
	};

    /* STRUCTURES */
#pragma pack(push, 1)
    struct TNumScanResults
    {
        unsigned char value;
    };

    struct TScan
    {
        char            ssid[MAX_SSID_LEN];
        unsigned char   mode;
        unsigned char   rssi;
    };

    struct TBssid
    {
        char            ssid[MAX_SSID_LEN];
        unsigned char   bssid[MAC_ADDRESS_LEN];
    };

    struct TNetworkType
    {
        char            ssid[MAX_SSID_LEN];
        unsigned char   nwType;

    };

#if 0
	struct TFeatureSelect
	{
		unsigned int	bit31	            : 1;
		unsigned int	bit30	            : 1;
		unsigned int	bit29	            : 1;
		unsigned int	bit28	            : 1;
		unsigned int	bit27	            : 1;
		unsigned int	bit26	            : 1;
		unsigned int	bit25	            : 1;
		unsigned int	bit24	            : 1;
		unsigned int	bit23	            : 1;
		unsigned int	bit22	            : 1;
		unsigned int	bit21	            : 1;
		unsigned int	bit20	            : 1;
		unsigned int	bit19	            : 1;
		unsigned int	bit18	            : 1;
		unsigned int	bit17	            : 1;
		unsigned int	bit16	            : 1;
		unsigned int	bit15	            : 1;
		unsigned int	bit14	            : 1;
		unsigned int	bit13	            : 1;
		unsigned int	bit12	            : 1;
		unsigned int	authModeRelevance   : 1;
		unsigned int	bit10	            : 1;
		unsigned int	bit9	            : 1;
		unsigned int	bit8	            : 1;
		unsigned int	wepInConfig         : 1;
		unsigned int	bit6	            : 1;
		unsigned int	bit5	            : 1;
		unsigned int	bit4	            : 1;
		unsigned int	pskLength           : 1;
		unsigned int	bit2	            : 1;
		unsigned int	bit1	            : 1;
		unsigned int	dnsServerAddr       : 1;
	};
#endif

    struct TIPConfig
    {
        unsigned char   mac[MAC_ADDRESS_LEN];
        unsigned char   address[NW_ADDRESS_LEN];
        unsigned char   subnet[NW_ADDRESS_LEN];
        unsigned char   gateway[NW_ADDRESS_LEN];
    };

    struct TSocket
    {
        unsigned char   id;
    };

    struct TSocketStatus
    {
        unsigned char   id;
        unsigned char   address[NW_ADDRESS_LEN];
        unsigned short  port;                       /*! @note Big endian */
    };

    struct TSend
    {
        unsigned char   ignore;
    };

    /*! @todo Fill something for READ? */
    struct TRead
    {
        unsigned char   socketId;
        unsigned short  size;
        unsigned char   address[NW_ADDRESS_LEN];
        unsigned short  srcPort;                    /*! @note Big endian */
        char           *data;
    };

    struct TDNSGet
    {
        unsigned char   numIPs;
        unsigned char   address1[NW_ADDRESS_LEN];
        unsigned char   address2[NW_ADDRESS_LEN];
        unsigned char   address3[NW_ADDRESS_LEN];
        unsigned char   address4[NW_ADDRESS_LEN];
        unsigned char   address5[NW_ADDRESS_LEN];
        unsigned char   address6[NW_ADDRESS_LEN];
        unsigned char   address7[NW_ADDRESS_LEN];
        unsigned char   address8[NW_ADDRESS_LEN];
        unsigned char   address9[NW_ADDRESS_LEN];
        unsigned char   address10[NW_ADDRESS_LEN];
    };

    struct TFWVersion
    {
        char            version[5];
    };

    struct TNetworkParams
    {
        char            ssid[MAX_SSID_LEN];
        unsigned char   secMode;
        char            psk[MAX_PSK_LEN];
        unsigned char   channel;
        unsigned char   mac[MAC_ADDRESS_LEN];
        unsigned char   dhcpMode;
        unsigned char   address[NW_ADDRESS_LEN];
        unsigned char   subnet[NW_ADDRESS_LEN];
        unsigned char   gateway[NW_ADDRESS_LEN];
        unsigned char   numOpenSockets;
        /*!note Socket Details. As many as numOpenSockets indicates. */
    };

    struct TSocketDetails
    {
        unsigned char   id;
        unsigned char   type;
        unsigned short  srcPort;                    /*! @note Big endian */
        unsigned short  dstPort;                    /*! @note Big endian */
        unsigned char   dstAddress[NW_ADDRESS_LEN];

    };

    struct TMACAddress
    {
        unsigned char   mac[MAC_ADDRESS_LEN];
    };

    struct TRSSI
    {
        unsigned char   value;
    };

    struct TStoredConfig
    {
        unsigned char   isValid;
        unsigned char   channel;
        unsigned char   nwType;
        unsigned char   secMode;
        unsigned char   dataRate;
        unsigned char   powerLevel;
        char            psk[MAX_PSK_LEN + 1];
        unsigned char   ssid[MAX_SSID_LEN];
        unsigned char   reserved;
        unsigned char   dhcp;
        unsigned char   address[NW_ADDRESS_LEN];
        unsigned char   subnet[NW_ADDRESS_LEN];
        unsigned char   gateway[NW_ADDRESS_LEN];
        unsigned char   featureSelect[4];
        /*! @note WEP Configuration only if Bit[7] of "Feature Select" is set to 1. */
    };

    struct TWEPConfig
    {
        unsigned char   authMode;
        unsigned char   index;
        unsigned char   keys[96];
    };
#pragma pack(pop)


    /* METHODS */
    RS9110_UART (IPersistor *persistor);
    ~RS9110_UART ();

    void            SetPersistor            (IPersistor *persistor);
    IPersistor *    GetPersistor            ();

    bool            ProcessMessage (char *message);

    ECommand        GetLastCommand          ();
    EResponseType   GetResponseType         ();
    EErrorCode      GetErrorCode            ();

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
    
	bool			PowerMode				(EPowerMode powerMode);
	bool			KeepSleeping			();
	bool			SetSleepTimer			(unsigned int milliseconds);

	bool			SetFeatureSelect		(unsigned int value);

    bool            IPConfiguration         (EDHCPMode eDHCPMode, const char *ipAddr, const char *subNetwork, const char *gateway);
	bool            OpenTcpSocket           (const char *hostIpAddr, unsigned short targetPort, unsigned short localPort);
    bool            OpenListeningUdpSocket  (unsigned short localPort);
    bool            OpenUdpSocket           (const char *hostIpAddr, unsigned short targetPort, unsigned short localPort);
    bool            OpenListeningTcpSocket  (unsigned short localPort);
    bool            GetSocketStatus         (unsigned char socketId);
	bool            CloseSocket             (unsigned char socketId);
    unsigned int    Send                    (unsigned char socketId, ESocketType socketType, const char *hostIpAddr, unsigned short hostPort, const char *data, unsigned int dataSize);
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
    void ProcessResponseType    (const char *message);
    bool IsValidSocketId        (unsigned char socketId);
    bool IsValidLocalTcpPort    (unsigned short port);
    bool GenericCommand         (ECommand command);
    bool GenericCommandInt      (ECommand command, int value);
    bool GenericCommandStr      (ECommand command, const char *str);


    /* VARIABLES */
    IPersistor     *_persistor;
    char            _outBuffer[MAX_BUFFER_SIZE];
    ECommand        _lastCommand;
    EResponseType   _responseType;
    EErrorCode      _errorCode;
};

#endif /* _RS9110_UART_H_ */
