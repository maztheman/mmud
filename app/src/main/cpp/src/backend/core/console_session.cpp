#include "console_session.h"

#include <mutex>

using write_lock = std::unique_lock<std::shared_mutex>;
using read_lock = std::shared_lock<std::shared_mutex>;


namespace kms {


console_session_t::~console_session_t()
{
    fmt::print(stderr, "ending console {}\n", m_sessionName);
}

void console_session_t::writeText(std::string text)
{
    m_texts.add(std::move(text));
    m_scrollToBottom = true;
}

console_session_t::ro_strings& console_session_t::readText(ro_strings& texts) const
{
    return m_texts.get(texts);
}

void console_session_t::setLocalEcho(bool bValue)
{
    m_localEcho = bValue;
}

void console_session_t::setInput(std::string input)
{
    if (m_localEcho)
    {
        writeText(input);
    }
    inputQueue.enqueue(input);
}

}