#include "RS9110_UART.h"

#include "IPersistor.h"

#include <string.h>
#include <stdio.h>


static const char *COMMAND[] =
{
    "AT+RSI_BAND=",
    "AT+RSI_INIT",
    "AT+RSI_NUMSCAN?",
    "AT+RSI_NUMSCAN=",
    "AT+RSI_SCAN=",
    "AT+RSI_NEXTSCAN",
    "AT+RSI_BSSID?",
    "AT+RSI_NWTYPE?",
    "AT+RSI_NETWORK=",
    "AT+RSI_PSK=",
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

static const char *CMD_ACK			= "ACK";
static const char *CMD_END          = "\r\n";

static const unsigned char CMD_RESP_OK_LEN      = strlen(CMD_RESP_OK);
static const unsigned char CMD_RESP_ERROR_LEN	= strlen(CMD_RESP_ERROR);
static const unsigned char CMD_RESP_READ_LEN    = strlen(CMD_RESP_READ);
static const unsigned char CMD_END_LEN          = strlen(CMD_END);


static bool         IsValidString       (const char *string, int maxLen = -1);
static unsigned int SendByteStuffing    (char *destination, unsigned int &dstSize, const char *source, unsigned int srcSize);



RS9110_UART::RS9110_UART (IPersistor *persistor)
  : _persistor(persistor),
    _lastCommand(CMD_MAX)
{
    memset(_outBuffer, 0, sizeof(_outBuffer));
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


IPersistor * RS9110_UART::GetPersistor ()
{
    return _persistor;
}


/*! @todo Should I return the response type? */
bool RS9110_UART::ProcessMessage (char *message)
{
    bool            bRtn = true;


    ProcessResponseType(message);

    _errorCode = ERROR_NONE;

    switch(GetResponseType())
    {
        case RESP_TYPE_OK:
            /*! @todo Stub */
        break;

        case RESP_TYPE_ERROR:
            if(strlen(message) > ((unsigned int) (CMD_RESP_ERROR_LEN + CMD_END_LEN)))
            {
                _errorCode = (EErrorCode) message[CMD_RESP_ERROR_LEN];
            }
        break;

        case RESP_TYPE_READ:
            /*! @todo Stub */
        break;

        case RESP_TYPE_CLOSE:
            /*! @todo Stub */
        break;

        case RESP_TYPE_SLEEP:
            /*! @todo Stub */
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
    if(eBand >= BAND_MAX)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

    return GenericCommandInt(CMD_BAND, eBand);
}


bool RS9110_UART::Init ()
{
    return GenericCommand(CMD_INIT);
}


bool RS9110_UART::GetNumScanResults ()
{
    return GenericCommand(CMD_GET_SCAN_RESULTS);
}


bool RS9110_UART::SetNumScanResults (unsigned char value)
{
    if((value <= 0) || (value > MAX_NUM_SCAN_RESULTS))
    {
        _lastCommand = CMD_MAX;
        return false;
    }

    return GenericCommandInt(CMD_SET_SCAN_RESULTS, value);
}


bool RS9110_UART::Scan (unsigned char channel, const char *ssid)
{
    bool bRtn;


    if(IsValidString(ssid) == false)
    {
        /*! @todo Check if we have to send '\0' or nothing */
        _snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%d%s", COMMAND[CMD_SCAN], channel, CMD_END);
    }
    else
    {
        if(strlen(ssid) > MAX_SSID_LEN)
        {
            _lastCommand = CMD_MAX;
            return false;
        }

        _snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%d,%s%s", COMMAND[CMD_SCAN], channel, ssid, CMD_END);
    }

    bRtn = _persistor->Write((unsigned char *) _outBuffer, strlen(_outBuffer));

    _lastCommand = ((bRtn == true) ? CMD_SCAN : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::NextScan ()
{
    return GenericCommand(CMD_NEXT_SCAN);
}


bool RS9110_UART::GetMACOfAPs ()
{
    return GenericCommand(CMD_GET_MAC_APS);
}


bool RS9110_UART::GetNetworkType ()
{
    return GenericCommand(CMD_GET_NETWORK_TYPE);
}


bool RS9110_UART::SetNetworkType (ENetworkType eNWType, EIBSSType eIBSSType, unsigned char channel)
{
    bool bRtn;


    switch(eNWType)
    {
        case NW_TYPE_INFRASTRUCTURE:
            _snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%s%s", COMMAND[CMD_SET_NETWORK_TYPE], NETWORK_TYPE_STR[eNWType], CMD_END);
        break;

        case NW_TYPE_IBSS:
        case NW_TYPE_IBSS_SEC:
            _snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%s,%d,%d%s", COMMAND[CMD_SET_NETWORK_TYPE], NETWORK_TYPE_STR[eNWType], eIBSSType, channel, CMD_END);
        break;

        default:
            _lastCommand = CMD_MAX;
            return false;
        break;
    }

    bRtn = _persistor->Write((unsigned char *) _outBuffer, strlen(_outBuffer));

    _lastCommand = ((bRtn == true) ? CMD_SET_NETWORK_TYPE : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::PSK (const char *psk)
{
    if(IsValidString(psk, MAX_PSK_LEN) == false)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

    return GenericCommandStr(CMD_PSK, psk);
}


bool RS9110_UART::AuthMode (EAuthMode eAuthMode)
{
    return GenericCommandInt(CMD_AUTH_MODE, eAuthMode);
}


bool RS9110_UART::Join (const char *ssid, ETxRate eTxRate, ETxPower eTxPower)
{
    bool bRtn;


    if(IsValidString(ssid, MAX_SSID_LEN) == false)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

    _snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%s,%d,%d%s", COMMAND[CMD_JOIN], ssid, eTxRate, eTxPower, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _outBuffer, strlen(_outBuffer));

    _lastCommand = ((bRtn == true) ? CMD_JOIN : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::Disassociate ()
{
    return GenericCommand(CMD_DISASSOCIATE);
}


bool RS9110_UART::PowerMode	(EPowerMode powerMode)
{
	return GenericCommandInt(CMD_POWER_MODE, powerMode);
}


bool RS9110_UART::KeepSleeping ()
{
	return GenericCommand(CMD_KEEP_SLEEPING);
}


bool RS9110_UART::SetSleepTimer (unsigned int milliseconds)
{
	if((milliseconds == 0) || (milliseconds > MAX_SLEEP_TIME_MS))
	{
		_lastCommand = CMD_MAX;
		return false;
	}

	return GenericCommandInt(CMD_SLEEP_TIMER, milliseconds);
}


/*! @todo The parameter should be an structure instead or reset all bits that must be '0' */
bool RS9110_UART::SetFeatureSelect (unsigned int value)
{
	return GenericCommandInt(CMD_FEATURE_SELECT, value);
}


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
				_lastCommand = CMD_MAX;
			}
			else
			{
				_snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%d,%s,%s,%s%s", COMMAND[CMD_IP_CONF], eDHCPMode, ipAddr, subNetwork, gateway, CMD_END);

				bRtn = _persistor->Write((unsigned char *) _outBuffer, strlen(_outBuffer));

				_lastCommand = ((bRtn == true) ? CMD_IP_CONF : CMD_MAX);
			}
		break;

		case DHCP_DHCP:
            /*! @todo Check if we have to send '0.0.0.0' or just '0' */
			_snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%d,0,0,0%s", COMMAND[CMD_IP_CONF], eDHCPMode, CMD_END);

			bRtn = _persistor->Write((unsigned char *) _outBuffer, strlen(_outBuffer));

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


	if((IsValidString(hostIpAddr) == false) || (IsValidLocalTcpPort(localPort) == false))
	{
        _lastCommand = CMD_MAX;
		return false;
	}

	_snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%s,%d,%d%s", COMMAND[CMD_OPEN_TCP_SOCKET], hostIpAddr, targetPort, localPort, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _outBuffer, strlen(_outBuffer));

	_lastCommand = ((bRtn == true) ? CMD_OPEN_TCP_SOCKET : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::OpenListeningUdpSocket  (unsigned short localPort)
{
    return GenericCommandInt(CMD_OPEN_LUDP_SOCKET, localPort);
}


bool RS9110_UART::OpenUdpSocket (const char *hostIpAddr, unsigned short targetPort, unsigned short localPort)
{
	bool bRtn;


	if(IsValidString(hostIpAddr) == false)
	{
        _lastCommand = CMD_MAX;
		return false;
	}

	_snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%s,%d,%d%s", COMMAND[CMD_OPEN_UDP_SOCKET], hostIpAddr, targetPort, localPort, CMD_END);

	bRtn = _persistor->Write((unsigned char *) _outBuffer, strlen(_outBuffer));

	_lastCommand = ((bRtn == true) ? CMD_OPEN_UDP_SOCKET : CMD_MAX);

	return bRtn;
}


bool RS9110_UART::OpenListeningTcpSocket  (unsigned short localPort)
{
    return GenericCommandInt(CMD_OPEN_LTCP_SOCKET, localPort);
}


bool RS9110_UART::GetSocketStatus (unsigned char socketId)
{
	if(IsValidSocketId(socketId) == false)
	{
        _lastCommand = CMD_MAX;
		return false;
	}

    return GenericCommandInt(CMD_GET_SOCKET_STATUS, socketId);
}


bool RS9110_UART::CloseSocket (unsigned char socketId)
{
	if(IsValidSocketId(socketId) == false)
	{
        _lastCommand = CMD_MAX;
		return false;
	}

    return GenericCommandInt(CMD_CLOSE_SOCKET, socketId);
}


unsigned int RS9110_UART::Send (unsigned char socketId, ESocketType socketType, const char *hostIpAddr, unsigned short hostPort, const char *data, unsigned int dataSize)
{
	bool         bRtn;
    unsigned int sendLen;
    unsigned int maxDataLen;


    if(IsValidSocketId(socketId) == false)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

    /* Command and Parameters */
    switch(socketType)
    {
        case SOCKET_TCP:
            _snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%d,0,0,0,", COMMAND[CMD_SEND_DATA], socketId);
            maxDataLen = MAX_SEND_DATA_SIZE_TCP;
        break;

        case SOCKET_UDP:
            _snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%d,0,%s,%d,", COMMAND[CMD_SEND_DATA], socketId, hostIpAddr, hostPort);
            maxDataLen = MAX_SEND_DATA_SIZE_UDP;
        break;

        default:
            _lastCommand = CMD_MAX;
            return 0;
        break;
    }

    /* Fill data after byte stuffing */
    unsigned int destSize = maxDataLen;
    unsigned int hdr = strlen(_outBuffer);
    sendLen = SendByteStuffing(&_outBuffer[strlen(_outBuffer)], destSize, data, dataSize);

    /* End of Command */
    _snprintf(&_outBuffer[hdr + destSize], sizeof(_outBuffer), "%s", CMD_END);

	bRtn = _persistor->Write((unsigned char *) _outBuffer, (hdr + destSize + strlen(CMD_END)));

	_lastCommand = ((bRtn == true) ? CMD_SEND_DATA : CMD_MAX);
	
    return sendLen;
}


bool RS9110_UART::GetDNS (const char *domainName)
{
    if(IsValidString(domainName) == false)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

    return GenericCommandStr(CMD_GET_DNS, domainName);
}


bool RS9110_UART::GetFirmwareVersion ()
{
    return GenericCommand(CMD_FW_VERSION);
}


bool RS9110_UART::GetNetworkParameters ()
{
    return GenericCommand(CMD_GET_NETWORK_PARAMS);
}

    
bool RS9110_UART::Reset ()
{
    return GenericCommand(CMD_RESET);
}


bool RS9110_UART::GetMACAddress ()
{
    return GenericCommand(CMD_GET_MAC);
}


bool RS9110_UART::GetRSSI ()
{
    return GenericCommand(CMD_GET_RSSI);
}


bool RS9110_UART::SaveConfiguration ()
{
    return GenericCommand(CMD_SAVE_CONFIG);
}


bool RS9110_UART::Configuration (EConfiguration eConfig)
{
    if(eConfig >= CONFIG_MAX)
    {
        _lastCommand = CMD_MAX;
        return false;
    }

    return GenericCommandInt(CMD_ENABLE_CONFIG, eConfig);
}


bool RS9110_UART::GetConfiguration ()
{
    return GenericCommand(CMD_GET_CONFIG);
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


inline bool RS9110_UART::IsValidSocketId (unsigned char socketId)
{
    return ((socketId >= MIN_SOCKET_HANDLE) && (socketId <= MAX_SOCKET_HANDLE));
}


inline bool RS9110_UART::IsValidLocalTcpPort (unsigned short port)
{
    return ((port >= MIN_TCP_SOCKET_PORT) && (port <= MAX_TCP_SOCKET_PORT));
}


bool RS9110_UART::GenericCommand (ECommand command)
{
    bool bRtn;


    _snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%s", COMMAND[command], CMD_END);

    bRtn = _persistor->Write((unsigned char *) _outBuffer, strlen(_outBuffer));

    _lastCommand = ((bRtn == true) ? command : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::GenericCommandInt (ECommand command, int value)
{
    bool bRtn;


    _snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%d%s", COMMAND[command], value, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _outBuffer, strlen(_outBuffer));

    _lastCommand = ((bRtn == true) ? command : CMD_MAX);

    return bRtn;
}


bool RS9110_UART::GenericCommandStr (ECommand command, const char *str)
{
    bool bRtn;


    _snprintf_s(_outBuffer, sizeof(_outBuffer), "%s%s%s", COMMAND[command], str, CMD_END);

    bRtn = _persistor->Write((unsigned char *) _outBuffer, strlen(_outBuffer));

    _lastCommand = ((bRtn == true) ? command : CMD_MAX);

    return bRtn;
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
            if(tmpSize < 8)
            {
                int i = 0;
                i++;
            }
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
