#include "RS9110_UART.h"

#include "IPersistor.h"

#include <string.h>
#include <stdio.h>


#if defined (WIN32)
#define _snprintf_s     _snprintf_s
#elif defined (AVR32)
#define _snprintf_s     snprintf
#define _snprintf       snprintf
#endif /* WIN32 */


static const char *COMMAND[] =
{
    "AT+RSI_BAND=",
    "AT+RSI_INIT",
    "AT+RSI_NUMSCAN?",
    "AT+RSI_NUMSCAN=",
    "AT+RSI_PASSSCAN=",
    "AT+RSI_SCAN=",
    "AT+RSI_NEXTSCAN",
    "AT+RSI_BSSID?",
    "AT+RSI_NWTYPE?",
    "AT+RSI_NETWORK=",
    "AT+RSI_PSK=",
    "AT+RSI_WEP_KEYS=",
    "AT+RSI_AUTHMODE=",
    "AT+RSI_JOIN=",
	"AT+RSI_DISASSOC",
	"AT+RSI_PWMODE=",
	"ACK",
	"AT+RSI_SLEEPTIMER=",
	"AT+RSI_FEAT_SEL=",
	"AT+RSI_IPCONF=",
	"AT+RSI_TCP=",
    "AT+RSI_LUDP=",
    "AT+RSI_UDP=",
    "AT+RSI_LTCP=",
    "AT+RSI_CTCP=",
	"AT+RSI_CLS=",
	"AT+RSI_SND=",
    "AT+RSI_DNSGET=",
	"AT+RSI_FWVERSION?",
    "AT+RSI_NWPARAMS?",
    "AT+RSI_RESET",
    "AT+RSI_MAC?",
    "AT+RSI_RSSI?",
	"AT+RSI_CFGSAVE",
	"AT+RSI_CFGENABLE=",
	"AT+RSI_CFGGET?"
};

static const char *NETWORK_TYPE_STR[] =
{
    "INFRASTRUCTURE",
    "IBSS",
    "IBSS_SEC"
};

static const char *CMD_RESP_OK      = "OK";
static const char *CMD_RESP_ERROR   = "ERROR";
static const char *CMD_RESP_READ    = "AT+RSI_READ";
static const char *CMD_RESP_CLOSE   = "AT+RSI_CLOSE";
static const char *CMD_RESP_SLEEP   = "SLEEP";

//static const char *CMD_ACK			= "ACK";
static const char *CMD_END          = "\r\n";

static const unsigned char CMD_RESP_OK_LEN      = strlen(CMD_RESP_OK);
static const unsigned char CMD_RESP_ERROR_LEN	= strlen(CMD_RESP_ERROR);
static const unsigned char CMD_RESP_READ_LEN    = strlen(CMD_RESP_READ);
static const unsigned char CMD_END_LEN          = strlen(CMD_END);


static bool         IsValidString       (const char *string, int maxLen = -1);
static unsigned int SendByteStuffing    (char *destination, unsigned int &dstSize, const char *source, unsigned int srcSize);



/*!
 *  @brief  Constructor
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *    Constructor.
 *
 *  @param[in]  persistor   - Pointer to the persistor
 *
 */
RS9110_UART::RS9110_UART (IPersistor *persistor)
  : _persistor(persistor),
    _responseLength(0),
    _lastCommand(CMD_MAX),
    _responseType(RESP_TYPE_MAX),
    _errorCode(ERROR_NONE)
{
    memset(_buffer, 0, sizeof(_buffer));
}


/*!
 *  @brief  Destructor
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *    Destructor.
 *
 */
RS9110_UART::~RS9110_UART ()
{
    /* Nothing to do */
}


/*!
 *  @brief  SetPersistor
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Set which persistor must be used (only Write method).
 *
 *  @param[in]  persistor   - Pointer to the persistor
 *
 */
void RS9110_UART::SetPersistor (IPersistor *persistor)
{
    if(persistor != NULL)
    {
        _persistor = persistor;
    }
}


/*!
 *  @brief  GetPersistor
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Get which persistor is used.
 *
 *  @return Pointer to the persistor
 *
 */
IPersistor * RS9110_UART::GetPersistor ()
{
    return _persistor;
}


/*!
 *  @brief  ProcessMessage
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Set which persistor must be used (only Write method).
 *
 *  @param[in]  message     - Pointer to the beginning of the incoming message
 *  @param[in]  size        - Size of the incoming message (in bytes)
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Response is not complete, too long or unknown
 */
bool RS9110_UART::ProcessMessage (char *message, int size)
{
    bool bRtn = true;


    if(strstr(&message[size - 2], CMD_END) == NULL)
    {
        _responseLength = -1;
        return false;
    }

    ProcessResponseType(message);

    _errorCode      = ERROR_NONE;
    _responseLength = 0;

    switch(GetResponseType())
    {
        case RESP_TYPE_OK:
            _responseLength = size - CMD_RESP_OK_LEN - CMD_END_LEN;

            if(_responseLength > 0)
            {
                if(_responseLength <= MAX_BUFFER_SIZE)
                {
                    memcpy(_buffer, &message[CMD_RESP_OK_LEN], _responseLength);
                }
                else
                {
                    _responseLength = -1;
                    bRtn = false;
                }
            }
        break;

        case RESP_TYPE_ERROR:
            if(size > (CMD_RESP_ERROR_LEN + CMD_END_LEN))
            {
                _errorCode = (EErrorCode) ((unsigned char) message[CMD_RESP_ERROR_LEN]);
            }
            else
            {
                bRtn = false;
            }
        break;

        case RESP_TYPE_READ:
            if(size <= MAX_BUFFER_SIZE)
            {
                _responseLength = size - CMD_RESP_READ_LEN - CMD_END_LEN;
                memcpy(_buffer, &message[CMD_RESP_READ_LEN], _responseLength);
            }
            else
            {
                _responseLength = -1;
                bRtn = false;
            }
        break;

        case RESP_TYPE_CLOSE:
            /* Nothing to do */
        break;

        case RESP_TYPE_SLEEP:
            /* Nothing to do */
        break;

        default:
            _responseLength = -1;
            bRtn = false;
        break;
    }

    return bRtn;
}


/*!
 *  @brief  GetResponseType
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Returns the type of resopnse received.
 *
 *  @return EResponseType
 */
RS9110_UART::EResponseType RS9110_UART::GetResponseType ()
{
    return _responseType;
}


/*!
 *  @brief  GetLastCommand
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Returns the last command sent.
 *
 *  @return ECommand
 */
RS9110_UART::ECommand RS9110_UART::GetLastCommand ()
{
    return _lastCommand;
}


int RS9110_UART::GetResponse (void *respBuffer)
{
    if((respBuffer != NULL) && (_responseType != RESP_TYPE_MAX))
    {
        respBuffer = _buffer;
    }

    return _responseLength;
}


/*!
 *  @brief  GetErrorCode
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Returns the code of the error. Valid value only if #RS9110_UART::GetResponseType returns #RESP_TYPE_ERROR.
 *
 *  @return ECommand
 */
RS9110_UART::EErrorCode RS9110_UART::GetErrorCode ()
{
    return _errorCode;
}


/*!
 *  @brief  Band
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command configures the band in which the module has to be configured.
 *
 *  @param[in]  eBand     - Type of band
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::Band (EBand eBand)
{
    if(eBand >= BAND_MAX)
    {
        SetLastCommand(CMD_MAX);
        return false;
    }

    return GenericCommandInt(CMD_BAND, eBand);
}


/*!
 *  @brief  Init
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command programs the RS9110-N-11-2X module's Baseband and RF components.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::Init ()
{
    return GenericCommand(CMD_INIT);
}


/*!
 *  @brief  GetNumScanResults
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is to get the number of networks already scanned by the "Scan"
 *      or "Next Scan" command.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::GetNumScanResults ()
{
    return GenericCommand(CMD_GET_SCAN_RESULTS);
}


/*!
 *  @brief  SetNumScanResults
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command configures the number of scan results the module returns for
 *      the Scan and NextScan commands.
 *
 *  @param[in]  value   - Number of scan results (Min = 1, Max = #MAX_NUM_SCAN_RESULTS)
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::SetNumScanResults (unsigned char value)
{
    if((value <= 0) || (value > MAX_NUM_SCAN_RESULTS))
    {
        SetLastCommand(CMD_MAX);
        return false;
    }

    return GenericCommandInt(CMD_SET_SCAN_RESULTS, value);
}


/*!
 *  @brief  PassiveScan
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command enables the module to scan for networks without sending a probe
 *      request from the module.
 *
 *  @param[in]  channels    - Channels passive scan is to be done
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::PassiveScan (unsigned int channels)
{
    return GenericCommandInt(CMD_PASSIVE_SCAN, (unsigned int) channels);
}


/*!
 *  @brief  Scan
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command scans in all the channels or the channel specified in the command #RS9110_UART::SetNumScanResults.
 *
 *  @param[in]  channels    - Channel Number
 *  @param[in]  ssid        - SSID (Max length is #MAX_SSID_LEN)
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::Scan (unsigned char channel, const char *ssid)
{
    bool bRtn;


    if(IsValidString(ssid) == false)
    {
        _snprintf_s(_buffer, sizeof(_buffer), "%s%d%s", COMMAND[CMD_SCAN], channel, CMD_END);
    }
    else
    {
        if(strlen(ssid) > MAX_SSID_LEN)
        {
            SetLastCommand(CMD_MAX);
            return false;
        }

        _snprintf_s(_buffer, sizeof(_buffer), "%s%d,%s%s", COMMAND[CMD_SCAN], channel, ssid, CMD_END);
    }

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    SetLastCommand(CMD_SCAN, bRtn);

    return bRtn;
}


/*!
 *  @brief  NextScan
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command returns the number of results (set by the command RS9110_UART::SetNumScanResults)
 *      obtained by #RS9110_UART::Scan.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::NextScan ()
{
    return GenericCommand(CMD_NEXT_SCAN);
}


/*!
 *  @brief  GetMACOfAPs
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command returns the MAC addresses of the Access Points returned by the "Scan" command.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::GetMACOfAPs ()
{
    return GenericCommand(CMD_GET_MAC_APS);
}


/*!
 *  @brief  GetNetworkType
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command returns the network type of the scanned AP.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::GetNetworkType ()
{
    return GenericCommand(CMD_GET_NETWORK_TYPE);
}


/*!
 *  @brief  SetNetworkType
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command configures the type of network the module wishes to join.
 *
 *  @param[in]  eNWType     - Type of the network to be created/joined
 *  @param[in]  eIBSSType   - Type of IBSS network (optional if #NW_TYPE_INFRASTRUCTURE is chosen)
 *  @param[in]  channel     - Channel in which the IBSS network has to be created (optional if #NW_TYPE_INFRASTRUCTURE is chosen)
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::SetNetworkType (ENetworkType eNWType, EIBSSType eIBSSType, unsigned char channel)
{
    bool bRtn;


    switch(eNWType)
    {
        case NW_TYPE_INFRASTRUCTURE:
            _snprintf_s(_buffer, sizeof(_buffer), "%s%s%s", COMMAND[CMD_SET_NETWORK_TYPE], NETWORK_TYPE_STR[eNWType], CMD_END);
        break;

        case NW_TYPE_IBSS:
        case NW_TYPE_IBSS_SEC:
            _snprintf_s(_buffer, sizeof(_buffer), "%s%s,%d,%d%s", COMMAND[CMD_SET_NETWORK_TYPE], NETWORK_TYPE_STR[eNWType], eIBSSType, channel, CMD_END);
        break;

        default:
            SetLastCommand(CMD_MAX);
            return false;
        break;
    }

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    SetLastCommand(CMD_SET_NETWORK_TYPE, bRtn);

    return bRtn;
}


/*!
 *  @brief  PSK
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command configures the PSK (Pre Shared Key) that is used for creating
 *      secured access.
 *
 *  @param[in]  psk - Preshared key (Max length is #MAX_PSK_LEN)
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::PSK (const char *psk)
{
    if(IsValidString(psk, MAX_PSK_LEN) == false)
    {
        SetLastCommand(CMD_MAX);
        return false;
    }

    return GenericCommandStr(CMD_PSK, psk);
}


/*!
 *  @brief  SetWEPKeys
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is issued to supply WEP keys to the module, if the AP is in WEP mode.
 *
 *  @param[in]  keyIndex    - Key index to be used to select the key.
 *  @param[in]  key2        - 2nd key of AP
 *  @param[in]  key3        - 3rd key of AP
 *  @param[in]  key4        - 4th key of AP
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::SetWEPKeys (unsigned char keyIndex, char *key2, char *key3, char *key4)
{
    bool bRtn;


    if((keyIndex > 3) ||
       (IsValidString(key2, MAX_PSK_LEN) == false) ||
       (IsValidString(key3, MAX_PSK_LEN) == false) ||
       (IsValidString(key4, MAX_PSK_LEN) == false))
    {
        SetLastCommand(CMD_MAX);
        return false;
    }

    _snprintf_s(_buffer, sizeof(_buffer), "%s%d,%s,%s,%s%s", COMMAND[CMD_WEP_KEYS], keyIndex, key2, key3, key4, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    SetLastCommand(CMD_WEP_KEYS, bRtn);

    return bRtn;
}


/*!
 *  @brief  AuthMode
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command configures the mode of security during the WLAN connection setup.
 *
 *  @param[in]  eAuthMode   - Authentication mode
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::AuthMode (EAuthMode eAuthMode)
{
    return GenericCommandInt(CMD_AUTH_MODE, eAuthMode);
}


/*!
 *  @brief  Join
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is used to join a network.
 *
 *  @param[in]  ssid        - SSID
 *  @param[in]  eTxRate     - Transmit Power level
 *  @param[in]  eTxPower    - Rate at which the data has to be transmitted
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::Join (const char *ssid, ETxRate eTxRate, ETxPower eTxPower)
{
    bool bRtn;


    if(IsValidString(ssid, MAX_SSID_LEN) == false)
    {
        SetLastCommand(CMD_MAX);
        return false;
    }

    _snprintf_s(_buffer, sizeof(_buffer), "%s%s,%d,%d%s", COMMAND[CMD_JOIN], ssid, eTxRate, eTxPower, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    SetLastCommand(CMD_JOIN, bRtn);

    return bRtn;
}


/*!
 *  @brief  Disassociate
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is issued to request the module to disassociate (disconnect)
 *      from an Access Point.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::Disassociate ()
{
    return GenericCommand(CMD_DISASSOCIATE);
}


/*!
 *  @brief  PowerMode
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command configures the power mode.
 *
 *  @param[in]  powerMode   - Power mode
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::PowerMode	(EPowerMode powerMode)
{
	return GenericCommandInt(CMD_POWER_MODE, powerMode);
}


/*!
 *  @brief  KeepSleeping
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This is not actually a command. This is an ACK to the module which must be sent as a response to the
 *      #RESP_TYPE_SLEEP response type. Sending this message back to the module makes it go to sleep again.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::KeepSleeping ()
{
	return GenericCommand(CMD_KEEP_SLEEPING);
}


/*!
 *  @brief  SetSleepTimer
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command configures the sleep timer which is used in Power Mode 1 when
 *      the module has not established the Wi-Fi connection.
 *
 *  @param[in]  milliseconds    - Time the module goes to sleep
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::SetSleepTimer (unsigned int milliseconds)
{
	if((milliseconds == 0) || (milliseconds > MAX_SLEEP_TIME_MS))
	{
		//_lastCommand = CMD_MAX;
        //_responseType = RESP_TYPE_MAX;
        SetLastCommand(CMD_MAX);
		return false;
	}

	return GenericCommandInt(CMD_SLEEP_TIMER, milliseconds);
}


/*!
 *  @brief  SetFeatureSelect
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is used to enable some configurable features.
 *
 *  @param[in]  value   - Configurable features
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::SetFeatureSelect (TFeatureSelect value)
{
    return GenericCommandInt(CMD_FEATURE_SELECT, (unsigned int) value.value);
}


/*!
 *  @brief  IPConfiguration
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command configures the IP address, subnet mask and default gateway of the TCP/IP stack.
 *
 *  @param[in]  eDHCPMode   - IP address in manual or DHCP mode
 *  @param[in]  ipAddr      - IP address in dotted decimal format
 *  @param[in]  subNetwork  - Subnet mask in dotted decimal format
 *  @param[in]  gateway     - Gateway in the dotted decimal format
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::IPConfiguration (EDHCPMode eDHCPMode, const char *ipAddr, const char *subNetwork, const char *gateway)
{
	bool bRtn = false;


	switch(eDHCPMode)
	{
		case DHCP_MANUAL:
			if((IsValidString(ipAddr)		== false) ||
			   (IsValidString(subNetwork)	== false) ||
			   (IsValidString(gateway)		== false))
			{
                SetLastCommand(CMD_MAX);
			}
			else
			{
				_snprintf_s(_buffer, sizeof(_buffer), "%s%d,%s,%s,%s%s", COMMAND[CMD_IP_CONF], eDHCPMode, ipAddr, subNetwork, gateway, CMD_END);

				bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

                SetLastCommand(CMD_IP_CONF, bRtn);
			}
		break;

		case DHCP_DHCP:
			_snprintf_s(_buffer, sizeof(_buffer), "%s%d,0,0%s", COMMAND[CMD_IP_CONF], eDHCPMode, CMD_END);

			bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

            SetLastCommand(CMD_IP_CONF, bRtn);
		break;

		default:
            SetLastCommand(CMD_MAX);
		break;
	}

	return bRtn;
}


/*!
 *  @brief  OpenTcpSocket
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command opens a client TCP socket and attempts to connect it to the
 *      specified "port" on a server defined by "host".
 *
 *  @param[in]  hostIpAddr  - Destination IP Address of the target server
 *  @param[in]  targetPort  - The target port
 *  @param[in]  localPort   - Local Port on the RS9110-N-11-2X module (allowed range #MIN_TCP_SOCKET_PORT to #MAX_TCP_SOCKET_PORT)
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::OpenTcpSocket (const char *hostIpAddr, unsigned short targetPort, unsigned short localPort)
{
	bool bRtn;


	if((IsValidString(hostIpAddr) == false) || (IsValidLocalTcpPort(localPort) == false))
	{
        SetLastCommand(CMD_MAX);
		return false;
	}

	_snprintf_s(_buffer, sizeof(_buffer), "%s%s,%d,%d%s", COMMAND[CMD_OPEN_TCP_SOCKET], hostIpAddr, targetPort, localPort, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    SetLastCommand(CMD_OPEN_TCP_SOCKET, bRtn);

	return bRtn;
}


/*!
 *  @brief  OpenListeningUdpSocket
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command opens a User Datagram Protocol (UDP) socket and binds to a specified port.
 *
 *  @param[in]  localPort   - Local port on the module
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::OpenListeningUdpSocket  (unsigned short localPort)
{
    return GenericCommandInt(CMD_OPEN_LUDP_SOCKET, localPort);
}


/*!
 *  @brief  OpenUdpSocket
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command opens a User Datagram Protocol (UDP) socket and links to the
 *      remote system's specific host and port address.
 *
 *  @param[in]  hostIpAddr   - IP Address of the Target server
 *  @param[in]  targetPort   - Target port (0 to 65535)
 *  @param[in]  localPort    - Local port on the module
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::OpenUdpSocket (const char *hostIpAddr, unsigned short targetPort, unsigned short localPort)
{
	bool bRtn;


	if(IsValidString(hostIpAddr) == false)
	{
        SetLastCommand(CMD_MAX);
		return false;
	}

	_snprintf_s(_buffer, sizeof(_buffer), "%s%s,%d,%d%s", COMMAND[CMD_OPEN_UDP_SOCKET], hostIpAddr, targetPort, localPort, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    SetLastCommand(CMD_OPEN_UDP_SOCKET, bRtn);

	return bRtn;
}


/*!
 *  @brief  OpenListeningTcpSocket
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This commands opens a TCP listening socket on the local IP address and
 *      the specified "port".
 *
 *  @param[in]  localPort    - Local port on the module
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::OpenListeningTcpSocket  (unsigned short localPort)
{
    return GenericCommandInt(CMD_OPEN_LTCP_SOCKET, localPort);
}


/*!
 *  @brief  GetSocketStatus
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command retrieves handles of active socket connections established
 *      through the listening socket.
 *
 *  @param[in]  socketId    - TCP listening socket handle for an already open listening socket in the module
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::GetSocketStatus (unsigned char socketId)
{
	if(IsValidSocketId(socketId) == false)
	{
        SetLastCommand(CMD_MAX);
		return false;
	}

    return GenericCommandInt(CMD_GET_SOCKET_STATUS, socketId);
}


/*!
 *  @brief  CloseSocket
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command closes a TCP/UDP socket in the module.
 *
 *  @param[in]  socketId    - Socket handle of an already open socket
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::CloseSocket (unsigned char socketId)
{
	if(IsValidSocketId(socketId) == false)
	{
        SetLastCommand(CMD_MAX);
		return false;
	}

    return GenericCommandInt(CMD_CLOSE_SOCKET, socketId);
}


/*!
 *  @brief  CloseSocket
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command sends a byte stream to the socket specified by the socket handle.
 *
 *  @param[in]  socketId    - Socket handle of an already open socket
 *  @param[in]  socketType  - Socket type
 *  @param[in]  hostIpAddr  - Destination IP Address
 *  @param[in]  hostPort    - Destination Port
 *  @param[in]  data        - Byte stream
 *  @param[in]  dataSize    - Length of the byte stream
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
unsigned int RS9110_UART::Send (unsigned char socketId, ESocketType socketType, const char *hostIpAddr, unsigned short hostPort, const char *data, unsigned int dataSize)
{
	bool         bRtn;
    unsigned int sendLen;
    unsigned int maxDataLen;


    if(IsValidSocketId(socketId) == false)
    {
        SetLastCommand(CMD_MAX);
        return 0;
    }

    /* Command and Parameters */
    switch(socketType)
    {
        case SOCKET_TCP:
            _snprintf_s(_buffer, sizeof(_buffer), "%s%d,0,0,0,", COMMAND[CMD_SEND_DATA], socketId);
            maxDataLen = MAX_SEND_DATA_SIZE_TCP;
        break;

        case SOCKET_UDP:
            _snprintf_s(_buffer, sizeof(_buffer), "%s%d,0,%s,%d,", COMMAND[CMD_SEND_DATA], socketId, hostIpAddr, hostPort);
            maxDataLen = MAX_SEND_DATA_SIZE_UDP;
        break;

        default:
            SetLastCommand(CMD_MAX);
            return 0;
        break;
    }

    /* Fill data after byte stuffing */
    unsigned int destSize = maxDataLen;
    unsigned int hdr = strlen(_buffer);
    sendLen = SendByteStuffing(&_buffer[strlen(_buffer)], destSize, data, dataSize);

    /* End of Command */
    _snprintf(&_buffer[hdr + destSize], sizeof(_buffer), "%s", CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, (hdr + destSize + strlen(CMD_END)));

    SetLastCommand(CMD_SEND_DATA, bRtn);

    return sendLen;
}


/*!
 *  @brief  GetDNS
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is used to send the domain name of a website to the module
 *      to resolve the IP address.
 *
 *  @param[in]  domainName  - Domain name of the target website
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::GetDNS (const char *domainName)
{
    if(IsValidString(domainName) == false)
    {
        SetLastCommand(CMD_MAX);
        return false;
    }

    return GenericCommandStr(CMD_GET_DNS, domainName);
}


/*!
 *  @brief  GetFirmwareVersion
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is used to retrieve the firmware version in the module.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::GetFirmwareVersion ()
{
    return GenericCommand(CMD_FW_VERSION);
}


/*!
 *  @brief  GetNetworkParameters
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is used to retrieve the WLAN connection and IP parameters.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::GetNetworkParameters ()
{
    return GenericCommand(CMD_GET_NETWORK_PARAMS);
}


/*!
 *  @brief  Reset
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command acts as a software reset to the module.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::Reset ()
{
    return GenericCommand(CMD_RESET);
}


/*!
 *  @brief  GetMACAddress
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is used to retrieve the MAC address of the module.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::GetMACAddress ()
{
    return GenericCommand(CMD_GET_MAC);
}


/*!
 *  @brief  GetRSSI
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is used to get the signal strength of the Access Point or
 *      network that the module is connected to.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::GetRSSI ()
{
    return GenericCommand(CMD_GET_RSSI);
}


/*!
 *  @brief  SaveConfiguration
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is used to save the parameters of an access point to internal memory.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::SaveConfiguration ()
{
    return GenericCommand(CMD_SAVE_CONFIG);
}


/*!
 *  @brief  Configuration
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is used to enable or disable the feature of automatic joining
 *      to a pre-configured Access Point on power up.
 *
 *  @pram[in]   eConfig - Enable/Disable the auto-join
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::Configuration (EConfiguration eConfig)
{
    if(eConfig >= CONFIG_MAX)
    {
        SetLastCommand(CMD_MAX);
        return false;
    }

    return GenericCommandInt(CMD_ENABLE_CONFIG, eConfig);
}


/*!
 *  @brief  GetConfiguration
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      This command is used to get the configuration values that have been
 *      stored in the module's memory.
 *
 *  @return bool
 *  @retval true    - OK
 *  @retval false   - Wrong argument or command not sent
 */
bool RS9110_UART::GetConfiguration ()
{
    return GenericCommand(CMD_GET_CONFIG);
}


/*!
 *  @brief  SetLastCommand
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Help method to assign values to multiple variables.
 *
 *  @param[in]  command         - Command to be sent
 *  @param[in]  isTransmitted   - Was it transmitted thru the persistor?
 */
void RS9110_UART::SetLastCommand (ECommand command, bool isTransmitted)
{
	_lastCommand    = ((isTransmitted == true) ? command : CMD_MAX);
    _responseType   = RESP_TYPE_MAX;
    _responseLength = 0;
}


/*!
 *  @brief  SetLastCommand
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Parses the begginig of the incoming message looking for the type of message.
 *
 *  @param[in]  message - Incoming message
 */
void RS9110_UART::ProcessResponseType (const char *message)
{
    if(strstr(message, CMD_RESP_OK) == message)
    {
        _responseType = RESP_TYPE_OK;
    }
    else if(strstr(message, CMD_RESP_ERROR) == message)
    {
        _responseType = RESP_TYPE_ERROR;
    }
    else if(strstr(message, CMD_RESP_READ) == message)
    {
        _responseType = RESP_TYPE_READ;
    }
    else if(strstr(message, CMD_RESP_CLOSE) == message)
    {
        _responseType = RESP_TYPE_CLOSE;
    }
	else if(strstr(message, CMD_RESP_SLEEP) == message)
	{
		_responseType = RESP_TYPE_SLEEP;
	}
    else
    {
        _responseType = RESP_TYPE_MAX;
    }
}


/*!
 *  @brief  IsValidSocketId
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Checks whether the socket is within the valid values
 *      (Min = #MIN_SOCKET_HANDLE, Max = #MAX_SOCKET_HANDLE).
 *
 *  @param[in]  socketId - Socket handle
 *
 *  @return bool
 */
inline bool RS9110_UART::IsValidSocketId (unsigned char socketId)
{
    return ((socketId >= MIN_SOCKET_HANDLE) && (socketId <= MAX_SOCKET_HANDLE));
}


/*!
 *  @brief  IsValidLocalTcpPort
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Checks whether the TCP port is within the valid values
 *      (Min = #MIN_TCP_SOCKET_PORT, Max = #MAX_TCP_SOCKET_PORT).
 *
 *  @param[in]  socketId - Socket handle
 *
 *  @return bool
 */
inline bool RS9110_UART::IsValidLocalTcpPort (unsigned short port)
{
    return ((port >= MIN_TCP_SOCKET_PORT) && (port <= MAX_TCP_SOCKET_PORT));
}


/*!
 *  @brief  GenericCommand
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Generic command sender. No arguments are nedded.
 *      i.e. "AT+RSI_RESET\r\n"
 *
 *  @param[in]  command - Command type
 *
 *  @return bool
 */
bool RS9110_UART::GenericCommand (ECommand command)
{
    bool bRtn;


    _snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[command], CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    SetLastCommand(command, bRtn);

    return bRtn;
}


/*!
 *  @brief  GenericCommandInt
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Generic command sender. Integer value is needed as argument.
 *      i.e. "AT+RSI_CTCP=1\r\n"
 *
 *  @param[in]  command - Command type
 *  @param[in]  value   - Integer value
 *
 *  @return bool
 */
bool RS9110_UART::GenericCommandInt (ECommand command, int value)
{
    bool bRtn;


    _snprintf_s(_buffer, sizeof(_buffer), "%s%d%s", COMMAND[command], value, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    SetLastCommand(command, bRtn);

    return bRtn;
}


/*!
 *  @brief  GenericCommandStr
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Generic command sender. Byte array (zero-ended) is needed as argument.
 *      i.e. "AT+RSI_CTCP=1\r\n"
 *
 *  @param[in]  command - Command type
 *  @param[in]  str     - String argument
 *
 *  @return bool
 */
bool RS9110_UART::GenericCommandStr (ECommand command, const char *str)
{
    bool bRtn;


    _snprintf_s(_buffer, sizeof(_buffer), "%s%s%s", COMMAND[command], str, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    SetLastCommand(command, bRtn);

    return bRtn;
}


/*!
 *  @brief  IsValidString
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Checks whether the string (byte array zero-ended) is valid. That means the
 *      string is not NULL, is not empty and its length is not larger than
 *      the maximum (this last is optional).
 *
 *  @param[in]  string - String to analyze
 *  @param[in]  maxLen - Maximum length
 *
 *  @return bool
 */
static bool IsValidString (const char *string, int maxLen)
{
    bool bRtn = true;


    if(maxLen < 0)
    {
        if((string == NULL) || (strlen(string) == 0))
        {
            bRtn = false;
        }
    }
    else
    {
	    if((string == NULL) || (strlen(string) == 0) || (((int) strlen(string)) > maxLen))
	    {
		    bRtn = false;
	    }
    }

	return bRtn;
}


/*!
 *  @brief  SendByteStuffing
 *
 *  @details
 *  <b>Details:</b><p>
 *
 *      Replaces the following values according to the documentation,
 *      - 0x0A 0x0D -> 0xDB 0xDC
 *      - 0xDB      -> 0xDB 0xDD
 *      - 0xDB 0xDC -> 0xDB 0xDD 0xDC
 *
 *  @param[in]      destination - Pointer to the destination buffer
 *  @param[in,out]  dstSize     - Input as max length of the destination buffer. Output as length of the data added to the destination buffer
 *  @param[in]      source      - Pointer to the source buffer/array
 *  @param[in]      srcSize     - Length of the data in the source buffer/array
 *
 *  @return bool
 */
static unsigned int SendByteStuffing (char *destination, unsigned int &dstSize, const char *source, unsigned int srcSize)
{
	const char *pos     = source;
    int         tmpSize = (int) dstSize;


	while((tmpSize >= 2) && (srcSize >= 1))
	{
		if((*pos == (char) 0x0D) && (*(pos+1) == (char) 0x0A))
		{
			*destination++ = (char) 0xDB;
			*destination++ = (char) 0xDC;
			pos += 2;
			tmpSize -= 2;
			srcSize -= 2;
		}
		else if(*pos == (char) 0xDB)
		{
			*destination++ = (char) 0xDB;
			*destination++ = (char) 0xDD;
			pos++;
			tmpSize -= 2;
			srcSize -= 1;
		}
		else
		{
			*destination++ = *pos++;
			tmpSize--;
			srcSize--;
		}
	}

    dstSize -= tmpSize;

	return ((unsigned int) (pos - source));
}
