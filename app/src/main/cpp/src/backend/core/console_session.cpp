#include "console_session.h"
namespace kms {

console_session_t::~console_session_t()
{

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
        if (!input.ends_with("\r\n"))
        {
            writeText(input + "\r\n");
        }
        else
        {
            writeText(input);
        }
    }
    inputQueue.enqueue(input);
}

}