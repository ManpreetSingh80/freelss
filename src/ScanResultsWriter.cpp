/*
 ****************************************************************************
 *  Copyright (c) 2014 Uriah Liggett <hairu526@gmail.com>                   *
 *	This file is part of FreeLSS.                                           *
 *                                                                          *
 *  FreeLSS is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, either version 3 of the License, or       *
 *  (at your option) any later version.                                     *
 *                                                                          *
 *  FreeLSS is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with FreeLSS.  If not, see <http://www.gnu.org/licenses/>.       *
 ****************************************************************************
*/
#include "Main.h"
#include "ScanResultsWriter.h"
#include "NeutralFileWriter.h"
#include "PlyWriter.h"

namespace scanner
{

ScanResultsWriter::ScanResultsWriter() :				//initialise
	m_cs(),
	m_records(),
	m_nfFilename(""),
	m_plyFilename("")
{
	// Do nothing
}

void ScanResultsWriter::setBaseFilePath(const std::string& baseFilePath)		//set filenames
{
	m_cs.enter();
	m_nfFilename = baseFilePath + ".db";
	m_plyFilename = baseFilePath + ".ply";
	m_cs.leave();
}

void ScanResultsWriter::write(const NeutralFileRecord& record)		//push records in critical section
{
	m_cs.enter();
	m_records.push_back(record);
	m_cs.leave();
}

size_t ScanResultsWriter::getNumPendingRecords()			//return pending requests
{
	size_t numPending;

	m_cs.enter();
	numPending = m_records.size();
	m_cs.leave();

	return numPending;
}

void ScanResultsWriter::run()			//if m_record is not empty,write data to file
{
	std::string nfFilename, plyFilename;

	NeutralFileWriter m_nfWriter;
	PlyWriter m_plyWriter;

	m_cs.enter();
	nfFilename = m_nfFilename;
	plyFilename = m_plyFilename;
	m_cs.leave();

	try
	{
		m_nfWriter.open(nfFilename);
		m_plyWriter.begin(plyFilename.c_str());

		bool acquiredRecord = false;
		NeutralFileRecord record;

		m_nfWriter.beginBatch();
		int numWritten = 0;

		while (!Thread::m_stopRequested)
		{
			// Check to see if there is work to be done
			m_cs.enter();
			if (!m_records.empty())
			{
				record = m_records.front();
				m_records.pop_front();
				acquiredRecord = true;
			}
			else
			{
				acquiredRecord = false;
			}
			m_cs.leave();


			// If there is data, do work
			if (acquiredRecord)
			{
				m_nfWriter.write(record);
				m_plyWriter.writePoints(&record.point, 1);
				numWritten++;
			}
			else
			{
				Thread::usleep(1000);
			}

			// Write to the neutral file in batches
			if ((numWritten % 2000) == 0)
			{
				m_nfWriter.commit();
				m_nfWriter.beginBatch();
			}
		}
	}
	catch (Exception& ex)
	{
		std::cerr << "Error: " << ex << std::endl;
	}
	catch (...)
	{
		std::cerr << "!! Unknown error occurred!!!" << std::endl;
	}

	// Flush any remaining records
	m_nfWriter.commit();

	// Close the output files
	m_nfWriter.close();
	m_plyWriter.end();

}

}
