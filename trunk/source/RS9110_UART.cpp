#include "RS9110_UART.h"

#include "IPersistor.h"

#include <string.h>
#include <stdio.h>



static const char *COMMAND[] =
{
    "AT+RSI_BAND=",
    "AT+RSI_INIT",
    "AT+RSI_SCAN=",
    "AT+RSI_NETWORK=",
    "AT+RSI_PSK=",
    "AT+RSI_AUTHMODE=",
    "AT+RSI_JOIN=",
	"AT+RSI_RESET",
	"AT+RSI_IPCONF=",
	"AT+RSI_CLS=",
	"AT+RSI_TCP=",
	"AT+RSI_CFGSAVE",
	"AT+RSI_CFGENABLE=",
	"AT+RSI_CFGGET?",
	"AT+RSI_DISASSOC",
	"AT+RSI_FWVERSION?",
	"AT+RSI_SEND=",
    "AT+RSI_MAC?",
    "AT+RSI_RSSI?",
    "AT+RSI_NWPARAMS?",
    "AT+RSI_UDP=",
    "AT+RSI_LTCP=",
    "AT+RSI_LUDP=",
    "AT+RSI_CTCP=",
    "AT+RSI_NUMSCAN?",
    "AT+RSI_NUMSCAN=",
    "AT+RSI_NEXTSCAN",
    "AT+RSI_BSSID?",
    "AT+RSI_NWTYPE?",
    "AT+RSI_DNSGET="
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

static const char *CMD_END          = "\r\n";

static const unsigned char CMD_RESP_OK_LEN      = strlen(CMD_RESP_OK);
static const unsigned char CMD_RESP_ERROR_LEN	= strlen(CMD_RESP_ERROR);
static const unsigned char CMD_END_LEN          = strlen(CMD_END);
static const unsigned char CMD_READ_LEN         = strlen(CMD_RESP_READ);


static bool IsValidString (const char *string, int maxLen = -1);
static unsigned int SendByteStuffing (char *destination, unsigned int &dstSize, const char *source, unsigned int srcSize);



RS9110_UART::RS9110_UART (IPersistor *persistor)
  : _persistor(persistor),
    _lastCommand(CMD_MAX)
{
    memset(_buffer, 0, sizeof(_buffer));
}


RS9110_UART::~RS9110_UART ()
{
    /* Nothing to do */
}


void RS9110_UART::SetPersistor (IPersistor *persistor)
{
    if(persistor != NULL)
    {
        _persistor = persistor;
    }
}


/*! @todo Should I return the response type? */
bool RS9110_UART::ProcessMessage (char *message, unsigned int size)
{
    bool bRtn;


    ProcessResponseType(message);

    switch(GetResponseType())
    {
        case RESP_TYPE_OK:
            _errorCode = ERROR_NONE;

            if(size == ((unsigned int) (CMD_RESP_OK_LEN + CMD_END_LEN)))
            {
                return true;
            }
        break;

        case RESP_TYPE_ERROR:
            if(size > ((unsigned int) (CMD_RESP_ERROR_LEN + CMD_END_LEN)))
            {
                _errorCode = (EErrorCode) ((unsigned char) message[CMD_RESP_ERROR_LEN]);

                return true;
            }
        break;

        case RESP_TYPE_READ:
            /*! @todo Stub */
        break;

        case RESP_TYPE_CLOSE:
            /*! @todo Stub */
        break;

        default:
            return false;
        break;
    }

    /* Fetch information after OK response */
    switch(_lastCommand)
    {
        case CMD_BAND:
        case CMD_INIT:
            /* Do nothing */
        break;

        case CMD_SCAN:
        break;

        default:
            bRtn = false;
        break;
    }

    return bRtn;
}


RS9110_UART::EResponseType RS9110_UART::GetResponseType ()
{
    return _responseType;
}


RS9110_UART::ECommand RS9110_UART::GetLastCommand ()
{
    return _lastCommand;
}


RS9110_UART::EErrorCode RS9110_UART::GetErrorCode ()
{
    return _errorCode;
}


bool RS9110_UART::Band (EBand eBand)
{
    bool bRtn;


    if(eBand >= BAND_MAX)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

    _snprintf_s(_buffer, sizeof(_buffer), "%s%d%s", COMMAND[CMD_BAND], eBand, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_BAND : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::Init ()
{
    bool bRtn;


    _snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_INIT], CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_INIT : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::GetNumScanResults ()
{
    bool bRtn;


    _snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_GET_SCAN_RESULTS], CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_GET_SCAN_RESULTS : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::SetNumScanResults (unsigned char value)
{
    bool bRtn;


    if((value <= 0) || (value > MAX_NUM_SCAN_RESULTS))
    {
        return false;
    }

    _snprintf_s(_buffer, sizeof(_buffer), "%s%d%s", COMMAND[CMD_SET_SCAN_RESULTS], value, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_SET_SCAN_RESULTS : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::Scan (unsigned char channel, const char *ssid)
{
    bool bRtn;


    if(IsValidString(ssid) == false)
    {
        /*! @todo Check if we have to send '\0' or nothing */
        _snprintf_s(_buffer, sizeof(_buffer), "%s%d%s", COMMAND[CMD_SCAN], channel, CMD_END);
    }
    else
    {
        if(strlen(ssid) > MAX_SSID_LEN)
        {
            _lastCommand = CMD_MAX;
            return false;
        }

        _snprintf_s(_buffer, sizeof(_buffer), "%s%d,%s%s", COMMAND[CMD_SCAN], channel, ssid, CMD_END);
    }

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_SCAN : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::NextScan ()
{
    bool bRtn;


    _snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_NEXT_SCAN], CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_NEXT_SCAN : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::GetMACOfAPs ()
{
    bool bRtn;


    _snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_GET_MAC_APS], CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_GET_MAC_APS : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::GetNetworkType ()
{
    bool bRtn;


    _snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_GET_NETWORK_TYPE], CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_GET_NETWORK_TYPE : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::SetNetworkType (ENetworkType eNWType, EIBSSType eIBSSType, unsigned char channel)
{
    bool bRtn;


    switch(eNWType)
    {
        case NW_TYPE_INFRASTRUCTURE:
            _snprintf_s(_buffer, sizeof(_buffer), "%s%s%s", COMMAND[CMD_SET_NETWORK_TYPE], NETWORK_TYPE_STR[NW_TYPE_INFRASTRUCTURE], CMD_END);
        break;

        case NW_TYPE_IBSS:
            _snprintf_s(_buffer, sizeof(_buffer), "%s%s,%d,%d%s", COMMAND[CMD_SET_NETWORK_TYPE], NETWORK_TYPE_STR[NW_TYPE_IBSS], eIBSSType, channel, CMD_END);
        break;

        case NW_TYPE_IBSS_SEC:
            _snprintf_s(_buffer, sizeof(_buffer), "%s%s,%d,%d%s", COMMAND[CMD_SET_NETWORK_TYPE], NETWORK_TYPE_STR[NW_TYPE_IBSS_SEC], eIBSSType, channel, CMD_END);
        break;

        default:
            _lastCommand = CMD_MAX;
            return false;
        break;
    }

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_SET_NETWORK_TYPE : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::PSK (const char *psk)
{
    bool bRtn;


    if(IsValidString(psk, MAX_PSK_LEN) == false)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

    _snprintf_s(_buffer, sizeof(_buffer), "%s%s%s", COMMAND[CMD_PSK], psk, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_PSK : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::AuthMode (EAuthMode eAuthMode)
{
    bool bRtn;


    _snprintf_s(_buffer, sizeof(_buffer), "%s%d%s", COMMAND[CMD_AUTH_MODE], eAuthMode, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_AUTH_MODE : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::Join (const char *ssid, ETxRate eTxRate, ETxPower eTxPower)
{
    bool bRtn;


    if(IsValidString(ssid, MAX_SSID_LEN) == false)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

    _snprintf_s(_buffer, sizeof(_buffer), "%s%s,%d,%d%s", COMMAND[CMD_JOIN], ssid, eTxRate, eTxPower, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_JOIN : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::Disassociate ()
{
	bool bRtn;


	_snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_DISASSOCIATE], CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_DISASSOCIATE : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::IPConfiguration (EDHCPMode eDHCPMode, const char *ipAddr, const char *subNetwork, const char *gateway)
{
	bool bRtn = false;


    /*! @todo   I think I should not check the format of the IP addresses. That must be done by the user.
     *          The only think I must check is NULL and length = 0.
     */
	switch(eDHCPMode)
	{
		case DHCP_MANUAL:
			if((IsValidString(ipAddr)		== false) ||
			   (IsValidString(subNetwork)	== false) ||
			   (IsValidString(gateway)		== false))
			{
				_lastCommand = CMD_MAX;
			}
			else
			{
				_snprintf_s(_buffer, sizeof(_buffer), "%s%d,%s,%s,%s%s", COMMAND[CMD_IP_CONF], eDHCPMode, ipAddr, subNetwork, gateway, CMD_END);

				bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

				_lastCommand = ((bRtn == true) ? CMD_IP_CONF : CMD_MAX);
			}
		break;

		case DHCP_DHCP:
            /*! @todo Check if we have to send '0.0.0.0' or just '0' */
			_snprintf_s(_buffer, sizeof(_buffer), "%s%d,0,0,0%s", COMMAND[CMD_IP_CONF], eDHCPMode, CMD_END);

			bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

			_lastCommand = ((bRtn == true) ? CMD_IP_CONF : CMD_MAX);
		break;

		default:
			_lastCommand = CMD_MAX;
		break;
	}

	return bRtn;
}


bool RS9110_UART::OpenTcpSocket (const char *hostIpAddr, unsigned short targetPort, unsigned short localPort)
{
	bool bRtn;


    /*! @todo   I think I should not check the format of the IP addresses. That must be done by the user.
     *          The only think I must check is NULL and length = 0.
     */
	if((IsValidString(hostIpAddr) == false) ||
	   (!((localPort >= MIN_TCP_SOCKET_PORT) && (localPort <= MAX_TCP_SOCKET_PORT))))
	{
        _lastCommand = CMD_MAX;
		return false;
	}

	_snprintf_s(_buffer, sizeof(_buffer), "%s%s,%d,%d%s", COMMAND[CMD_OPEN_TCP_SOCKET], hostIpAddr, targetPort, localPort, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_OPEN_TCP_SOCKET : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::OpenListeningUdpSocket  (unsigned short localPort)
{
	bool bRtn;


	_snprintf_s(_buffer, sizeof(_buffer), "%s%d%s", COMMAND[CMD_OPEN_LISTENING_UDP_SOCKET], localPort, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_OPEN_LISTENING_UDP_SOCKET : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::OpenUdpSocket (const char *hostIpAddr, unsigned short targetPort, unsigned short localPort)
{
	bool bRtn;


    /*! @todo   I think I should not check the format of the IP addresses. That must be done by the user.
     *          The only think I must check is NULL and length = 0.
     */
	if(IsValidString(hostIpAddr) == false)
	{
        _lastCommand = CMD_MAX;
		return false;
	}

	_snprintf_s(_buffer, sizeof(_buffer), "%s%s,%d,%d%s", COMMAND[CMD_OPEN_UDP_SOCKET], hostIpAddr, targetPort, localPort, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_OPEN_UDP_SOCKET : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::OpenListeningTcpSocket  (unsigned short localPort)
{
	bool bRtn;


	_snprintf_s(_buffer, sizeof(_buffer), "%s%d%s", COMMAND[CMD_OPEN_LISTENING_TCP_SOCKET], localPort, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_OPEN_LISTENING_TCP_SOCKET : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::GetSocketStatus (unsigned char socketId)
{
	bool bRtn;


	if(IsValidSocketId(socketId) == false)
	{
        _lastCommand = CMD_MAX;
		return false;
	}

    _snprintf_s(_buffer, sizeof(_buffer), "%s%d%s", COMMAND[CMD_GET_SOCKET_STATUS], socketId, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_GET_SOCKET_STATUS : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::CloseSocket (unsigned char socketId)
{
	bool bRtn;


	if(IsValidSocketId(socketId) == false)
	{
        _lastCommand = CMD_MAX;
		return false;
	}

	_snprintf_s(_buffer, sizeof(_buffer), "%s%d%s", COMMAND[CMD_CLOSE_SOCKET], socketId, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_CLOSE_SOCKET : CMD_MAX);

	return bRtn;
}


unsigned int RS9110_UART::Send (unsigned char socketId, const char *hostIpAddr, unsigned short hostPort, const char *data, unsigned int dataSize)
{
	bool bRtn;
    unsigned int sendLen;


    if(IsValidSocketId(socketId) == false)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

    /* Command and Parameters */
	_snprintf_s(_buffer, sizeof(_buffer), "%s%d,0,%s,%d,", COMMAND[CMD_SEND_DATA], socketId, hostIpAddr, hostPort);

    /* Fill data after byte stuffing */
    unsigned int destSize = sizeof(_buffer) - strlen(_buffer);
    unsigned int hdr = destSize;
    sendLen = SendByteStuffing(&_buffer[strlen(_buffer)], destSize, data, dataSize);

    /* End of Command */
    _snprintf(&_buffer[hdr + destSize], sizeof(_buffer), "%s", CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_SEND_DATA : CMD_MAX);
	
    return sendLen;
}


bool RS9110_UART::GetDNS (const char *domainName)
{
	bool bRtn;


    if(IsValidString(domainName) == false)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

	_snprintf_s(_buffer, sizeof(_buffer), "%s%s%s", COMMAND[CMD_GET_DNS], domainName, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_GET_DNS : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::GetFirmwareVersion ()
{
	bool bRtn;


	_snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_FW_VERSION], CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_FW_VERSION : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::GetNetworkParameters ()
{
	bool bRtn;


	_snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_GET_NETWORK_PARAMS], CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_GET_NETWORK_PARAMS : CMD_MAX);

	return bRtn;
}

    
bool RS9110_UART::Reset ()
{
	bool bRtn;


	_snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_RESET], CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_RESET : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::GetMACAddress ()
{
	bool bRtn;


	_snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_GET_MAC], CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_GET_MAC : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::GetRSSI ()
{
	bool bRtn;


	_snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_GET_RSSI], CMD_END);

    bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

    _lastCommand = ((bRtn == true) ? CMD_GET_RSSI : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::SaveConfiguration ()
{
	bool bRtn;


	_snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_SAVE_CONFIG], CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_SAVE_CONFIG : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::Configuration (EConfiguration eConfig)
{
	bool bRtn;


    if(eConfig >= CONFIG_MAX)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

	_snprintf_s(_buffer, sizeof(_buffer), "%s%d%s", COMMAND[CMD_ENABLE_CONFIG], eConfig, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_ENABLE_CONFIG : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::GetConfiguration ()
{
	bool bRtn;


	_snprintf_s(_buffer, sizeof(_buffer), "%s%s", COMMAND[CMD_GET_CONFIG], CMD_END);

	bRtn = _persistor->Write((unsigned char *) _buffer, strlen(_buffer));

	_lastCommand = ((bRtn == true) ? CMD_GET_CONFIG : CMD_MAX);

	return bRtn;
}


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
    else if (strstr(message, CMD_RESP_CLOSE) == message)
    {
        _responseType = RESP_TYPE_CLOSE;
    }
    else
    {
        _responseType = RESP_TYPE_UNKNOWN;
    }
}


inline bool RS9110_UART::IsValidSocketId (unsigned char socketId)
{
    return ((socketId >= MIN_SOCKET_HANDLE) && (socketId <= MAX_SOCKET_HANDLE));
}


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




static unsigned int SendByteStuffing (char *destination, unsigned int &dstSize, const char *source, unsigned int srcSize)
{
	const char     *pos     = source;
    unsigned int    tmpSize = dstSize;


	while((dstSize > 0) || (srcSize > 1))
	{
		if((*pos == 0x0D) && (*(pos+1) == 0x0A))
		{
			*destination++ = (char) 0xDB;
			*destination++ = (char) 0xDC;
			pos += 2;
			tmpSize -= 2;
			srcSize -= 2;
		}
		else if(*pos == 0xDB)
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
