/*
 * reader.cpp
 *
 *  Created on: Dec 13, 2022
 *      Author: max
 */

#include <reader.h>
#include <iostream>


GprWriter::GprWriter(asio::io_context& io) : m_context(io)
{
}

void GprWriter::write()
{
	std::cout << "Writing" << std::endl;
}
