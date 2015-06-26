#include "urlWorker.h"

#include <curl/curl.h>

static size_t write_data(void *_buffer, size_t _size, size_t _nmemb, void *_dataPtr) {
    rmt_ScopedCPUSample(read);
            
    const size_t realSize = _size * _nmemb;

    std::stringstream* stream = (std::stringstream*)_dataPtr;
    
    stream->write((const char*)_buffer, realSize);

    return realSize;
}

UrlWorker::UrlWorker() {
    m_curlHandle = curl_easy_init();
    curl_easy_setopt(m_curlHandle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(m_curlHandle, CURLOPT_WRITEDATA, &m_stream);
    curl_easy_setopt(m_curlHandle, CURLOPT_HEADER, 0L);
    curl_easy_setopt(m_curlHandle, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(m_curlHandle, CURLOPT_ACCEPT_ENCODING, "gzip");
    curl_easy_setopt(m_curlHandle, CURLOPT_TCP_NODELAY, 1);
    curl_easy_setopt(m_curlHandle, CURLOPT_BUFFERSIZE, 64 * 1024);
}

UrlWorker::~UrlWorker() {
    curl_easy_cleanup(m_curlHandle);
} 

void UrlWorker::perform(std::unique_ptr<UrlTask> _task) {
    
    m_task = std::move(_task);
    m_available = false;

    m_future = std::async(std::launch::async, [&]() {

        curl_easy_setopt(m_curlHandle, CURLOPT_URL, m_task->url.c_str());
        logMsg("Fetching URL with curl: %s\n", m_task->url.c_str());

        CURLcode result = curl_easy_perform(m_curlHandle);
        
        if (result != CURLE_OK) {
            logMsg("curl_easy_perform failed: %s\n", curl_easy_strerror(result));
        }

        size_t nBytes = m_stream.tellp();
        m_stream.seekp(0);

        m_task->content.resize(nBytes);
        m_stream.seekg(0);
        m_stream.read(m_task->content.data(), nBytes);

        m_finished = true;
        requestRender();
        return std::move(m_task);
    });
}

void UrlWorker::reset() {
    m_task.reset();
    m_available = true;
    m_finished = false;
}

bool UrlWorker::hasTask(const std::string& _url) {
    return (m_task->url == _url);
}

std::unique_ptr<UrlTask> UrlWorker::getResult() {
    return std::move( m_future.get() );
}

