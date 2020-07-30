#include "CWinHttp.h"


CWinHttp::CWinHttp()
	: m_hInternet(NULL)
	, m_hConnect(NULL)
	, m_hRequest(NULL)
	, m_nConnTimeout(5000)
	, m_nSendTimeout(5000)
	, m_nRecvTimeout(5000)
	, m_bHttps(FALSE)
	, m_params(NULL)
{
	Open();
}


CWinHttp::~CWinHttp()
{
	Release();
}

BOOL CWinHttp::Open()
{
	m_hInternet = ::WinHttpOpen(
		L"Microsoft Internet Explorer",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);
	if (NULL == m_hInternet)
		return FALSE;
	::WinHttpSetTimeouts(m_hInternet, 0, m_nConnTimeout, m_nSendTimeout, m_nRecvTimeout);
	return TRUE;
}

VOID CWinHttp::Release()
{
	if (m_hRequest)
	{
		::WinHttpCloseHandle(m_hRequest);
		m_hRequest = nullptr;
	}
	if (m_hConnect)
	{
		::WinHttpCloseHandle(m_hConnect);
		m_hConnect = nullptr;
	}
	if (m_hInternet)
	{
		::WinHttpCloseHandle(m_hInternet);
		m_hInternet = nullptr;
	}
}

BOOL CWinHttp::Connect(LPCWSTR lpUrl, REQUEST_TYPE type, LPCSTR lpPostData /*= NULL*/, LPCWSTR lpHeader /*= NULL*/)
{
	BOOL ret;
	wstring strHostName, strPage;
	WORD wPort;
	ParseUrlW(lpUrl, strHostName, strPage, wPort);
	if (wPort == INTERNET_DEFAULT_HTTPS_PORT)
		m_bHttps = TRUE;

	DWORD dwFlag = m_bHttps ? WINHTTP_FLAG_SECURE : 0;
	const wchar_t* pVerb = (type == Get) ? L"GET" : L"POST";

	do
	{
		m_hConnect = ::WinHttpConnect(m_hInternet, strHostName.c_str(), wPort, 0);
		if (!m_hConnect)
			break;

		m_hRequest = ::WinHttpOpenRequest(
			m_hConnect,
			pVerb,
			strPage.c_str(),
			NULL,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			dwFlag);
		if (!m_hRequest)
			break;

		if (m_bHttps)
		{
			DWORD buffer = SECURITY_FLAG_SECURE |
				SECURITY_FLAG_IGNORE_UNKNOWN_CA |
				SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
				SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
				SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
			ret = ::WinHttpSetOption(m_hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &buffer, sizeof(buffer));//不需要判断ret
		}

		DWORD dwSize = (NULL == lpPostData) ? 0 : strlen(lpPostData);
		if (lpHeader == NULL)
			ret = ::WinHttpSendRequest(m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)lpPostData, dwSize, dwSize, NULL);
		else
			ret = ::WinHttpSendRequest(m_hRequest, lpHeader, -1L, (LPVOID)lpPostData, dwSize, dwSize, NULL);
		if (!ret)
			break;

		ret = ::WinHttpReceiveResponse(m_hRequest, NULL);
		if (!ret)
			break;

		// SUCCESS!!!
		return TRUE;
	} while (false);

	return FALSE;
}

std::string CWinHttp::Request(LPCWSTR lpUrl, REQUEST_TYPE type, LPCSTR lpPostData /*= NULL*/, LPCWSTR lpHeader /*= NULL*/)
{
	std::string result;

	if (!Connect(lpUrl, type, lpPostData, lpHeader))
		return result;

	DWORD dwBytesToRead, dwReadSize;
	void* lpBuff = malloc(READ_BUFFER_SIZE);
	bool bFinish = false;

	while (true)
	{
		if (!::WinHttpQueryDataAvailable(m_hRequest, &dwBytesToRead))
			break;
		if (dwBytesToRead <= 0)
		{
			bFinish = true;
			break;
		}
		if (dwBytesToRead > READ_BUFFER_SIZE)
		{
			free(lpBuff);
			lpBuff = malloc(dwBytesToRead);
		}
		if (!::WinHttpReadData(m_hRequest, lpBuff, dwBytesToRead, &dwReadSize))
			break;
		result.append((const char*)lpBuff, dwReadSize);
	}

	free(lpBuff);
	if (!bFinish)
		result.clear();

	return result;
}

VOID CWinHttp::ParseUrlW(LPCWSTR lpUrl, wstring& strHostName, wstring& strPage, WORD& sPort)
{
	sPort = 80;
	wstring strTemp(lpUrl);
	int nPos = strTemp.find(L"http://");
	if (wstring::npos != nPos)
		strTemp = strTemp.substr(nPos + 7, strTemp.size() - nPos - 7);
	else
	{
		nPos = strTemp.find(L"https://");
		if (wstring::npos != nPos)
		{
			sPort = 443;//INTERNET_DEFAULT_HTTPS_PORT;
			strTemp = strTemp.substr(nPos + 8, strTemp.size() - nPos - 8);
		}
	}
	nPos = strTemp.find('/');
	if (wstring::npos == nPos)//没有找到 /
		strHostName = strTemp;
	else
		strHostName = strTemp.substr(0, nPos);
	int nPos1 = strHostName.find(':');
	if (nPos1 != wstring::npos)
	{
		wstring strPort = strTemp.substr(nPos1 + 1, strHostName.size() - nPos1 - 1);
		strHostName = strHostName.substr(0, nPos1);
		sPort = (WORD)_wtoi(strPort.c_str());
	}
	if (wstring::npos == nPos) {
		strPage = '/';
		return;
	}
	strPage = strTemp.substr(nPos, strTemp.size() - nPos);
}