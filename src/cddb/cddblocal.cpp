 /* fre:ac - free audio converter
  * Copyright (C) 2001-2014 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <cddb/cddblocal.h>
#include <bonkenc.h>
#include <config.h>

using namespace smooth::IO;

using namespace BoCA;
using namespace BoCA::AS;

BonkEnc::CDDBLocal::CDDBLocal()
{
	protocol = Protocol::Get("CDDB communication");
}

BonkEnc::CDDBLocal::~CDDBLocal()
{
}

Bool BonkEnc::CDDBLocal::QueryUnixDB(Int discid)
{
	static String	 array[11] = { "rock", "misc", "newage", "soundtrack", "blues", "jazz", "folk", "country", "reggae", "classical", "data" };

	BoCA::Config		*config = BoCA::Config::Get();
	Registry		&boca	= Registry::Get();
	DeviceInfoComponent	*info	= boca.CreateDeviceInfoComponent();

	if (info == NIL) return False;

	const MCDI	&mcdi = info->GetNthDeviceMCDI(activeDriveID);

	Int		 numTocEntries = mcdi.GetNumberOfEntries();
	Array<Int>	 discOffsets;
	Int		 discLength;

	for (Int l = 0; l < numTocEntries; l++) discOffsets.Add(mcdi.GetNthEntryOffset(l) + 150);

	discLength = mcdi.GetNthEntryOffset(numTocEntries) / 75 + 2;

	boca.DeleteComponent(info);

	String	 inputFormat = String::SetInputFormat("UTF-8");
	String	 outputFormat = String::SetOutputFormat("UTF-8");

	for (Int i = 0; i < 11; i++)
	{
		if (!File(String(config->GetStringValue(Config::CategoryFreedbID, Config::FreedbDirectoryID, Config::FreedbDirectoryDefault)).Append(array[i]).Append(Directory::GetDirectoryDelimiter()).Append(DiscIDToString(discid))).Exists()) continue;

		InStream	*in = new InStream(STREAM_FILE, String(config->GetStringValue(Config::CategoryFreedbID, Config::FreedbDirectoryID, Config::FreedbDirectoryDefault)).Append(array[i]).Append(Directory::GetDirectoryDelimiter()).Append(DiscIDToString(discid)), IS_READ);
		String		 result = in->InputString(in->Size());

		delete in;

		CDDBInfo	 cddbInfo;

		ParseCDDBRecord(result, cddbInfo);

		if (discLength == cddbInfo.discLength)
		{
			Bool	 match = True;

			for (Int j = 0; j < cddbInfo.trackOffsets.Length(); j++)
			{
				if (discOffsets.GetNth(j) != cddbInfo.trackOffsets.GetNth(j)) match = False;
			}

			if (match)
			{
				ids.Add(discid);
				categories.Add(array[i]);
				titles.Add(String(cddbInfo.dArtist).Append(" / ").Append(cddbInfo.dTitle));
				results.Add(result);
			}
		}
	}

	String::SetInputFormat(inputFormat);
	String::SetOutputFormat(outputFormat);

	return (results.Length() != 0);
}

Bool BonkEnc::CDDBLocal::QueryWinDB(Int discid)
{
	static String	 array[11] = { "rock", "misc", "newage", "soundtrack", "blues", "jazz", "folk", "country", "reggae", "classical", "data" };

	BoCA::Config		*config = BoCA::Config::Get();
	Registry		&boca	= Registry::Get();
	DeviceInfoComponent	*info	= boca.CreateDeviceInfoComponent();

	if (info == NIL) return False;

	const MCDI	&mcdi = info->GetNthDeviceMCDI(activeDriveID);

	Int		 numTocEntries = mcdi.GetNumberOfEntries();
	Array<Int>	 discOffsets;
	Int		 discLength;

	for (Int l = 0; l < numTocEntries; l++) discOffsets.Add(mcdi.GetNthEntryOffset(l) + 150);

	discLength = mcdi.GetNthEntryOffset(numTocEntries) / 75 + 2;

	boca.DeleteComponent(info);

	String	 inputFormat = String::SetInputFormat("UTF-8");
	String	 outputFormat = String::SetOutputFormat("UTF-8");

	for (Int i = 0; i < 11; i++)
	{
		Directory dir	  = Directory(String(config->GetStringValue(Config::CategoryFreedbID, Config::FreedbDirectoryID, Config::FreedbDirectoryDefault)).Append(array[i]));
		String	  pattern = String().CopyN(DiscIDToString(discid), 2).Append("to??");
		String	  found;

		do
		{
			const Array<File> &files = dir.GetFilesByPattern(pattern);

			if (files.Length() == 1) found = files.GetFirst();

			if	(pattern[1] == 'a') pattern[1] = '9';
			else if (pattern[1] == '0') pattern[1] = 'f';
			else			    pattern[1] -= 1;

			if (pattern[1] == 'f')
			{
				if	(pattern[0] == 'a') pattern[0] = '9';
				else if (pattern[0] == '0') pattern[0] = 'f';
				else			    pattern[0] -= 1;
			}
		}
		while (found == NIL && !(pattern[0] == 'f' && pattern[1] == 'f'));

		if (found == NIL) continue;

		InStream	*in = new InStream(STREAM_FILE, found, IS_READ);
		String		 idString = String("#FILENAME=").Append(DiscIDToString(discid));
		String		 result;

		while (in->GetPos() < in->Size())
		{
			if (in->InputLine() == idString)
			{
				Int	 start = in->GetPos();
				Int	 end = in->GetPos();

				while (in->GetPos() < in->Size())
				{
					end = in->GetPos();

					if (in->InputLine().StartsWith("#FILENAME=")) break;
				}

				in->Seek(start);

				result = in->InputString(end - start);

				break;
			}
		}

		delete in;

		if (result == NIL) continue;

		CDDBInfo	 cddbInfo;

		ParseCDDBRecord(result, cddbInfo);

		if (discLength == cddbInfo.discLength)
		{
			Bool	 match = True;

			for (Int j = 0; j < cddbInfo.trackOffsets.Length(); j++)
			{
				if (discOffsets.GetNth(j) != cddbInfo.trackOffsets.GetNth(j)) match = False;
			}

			if (match)
			{
				ids.Add(discid);
				categories.Add(array[i]);
				titles.Add(String(cddbInfo.dArtist).Append(" / ").Append(cddbInfo.dTitle));
				results.Add(result);
			}
		}
	}

	String::SetInputFormat(inputFormat);
	String::SetOutputFormat(outputFormat);

	return (results.Length() != 0);
}

Bool BonkEnc::CDDBLocal::ConnectToServer()
{
	return True;
}

Int BonkEnc::CDDBLocal::Query(Int discid)
{
	/* Try to find Unix style record first; if no match is found, try Windows style
	 */
	if (!QueryUnixDB(discid)) QueryWinDB(discid);

	/* No match found
	 */
	if (categories.Length() == 0) return QUERY_RESULT_NONE;

	/* Exact match
	 */
	if (categories.Length() == 1) return QUERY_RESULT_SINGLE;

	/* Multiple exact matches
	 */
	if (categories.Length() >  1) return QUERY_RESULT_MULTIPLE;

	return QUERY_RESULT_ERROR;
}

Int BonkEnc::CDDBLocal::Query(const String &queryString)
{
	/* Extract disc ID from query string and call Query() with disc ID
	 */
	String	 discID;

	for (Int i = 0; i < 8; i++) discID[i] = queryString[i + 11];

	return Query(StringToDiscID(discID));
}

Bool BonkEnc::CDDBLocal::Read(const String &category, Int discID, CDDBInfo &cddbInfo)
{
	cddbInfo.discID   = discID;
	cddbInfo.category = category;

	String	 result;

	for (Int i = 0; i < categories.Length(); i++) if (categories.GetNth(i) == cddbInfo.category) result = results.GetNth(i);

	if (result == NIL) return False;
	else		   return ParseCDDBRecord(result, cddbInfo);
}

Bool BonkEnc::CDDBLocal::Submit(const CDDBInfo &oCddbInfo)
{
	protocol->Write("Entering method: CDDBLocal::Submit(const CDDBInfo &)");

	BoCA::Config	*config = BoCA::Config::Get();

	CDDBInfo  cddbInfo = oCddbInfo;

	UpdateEntry(cddbInfo);

	String	  content = FormatCDDBRecord(cddbInfo);

	/* See if we have a Windows or Unix style DB
	 */
	Directory dir	  = Directory(String(config->GetStringValue(Config::CategoryFreedbID, Config::FreedbDirectoryID, Config::FreedbDirectoryDefault)).Append(cddbInfo.category));
	String	  pattern = String("??to??");

	/* Create directory if it doesn't exist
	 */
	dir.Create();

	if (dir.GetFilesByPattern(pattern).Length() >= 1) // Windows style DB
	{
		protocol->Write("Found Windows style DB.");

		pattern = String().CopyN(cddbInfo.DiscIDToString(), 2).Append("to??");

		String	  found;

		while (found == NIL && !(pattern[0] == '0' && pattern[1] == '0'))
		{
			const Array<File> &files = dir.GetFilesByPattern(pattern);

			if (files.Length() == 1) found = files.GetFirst();

			if (pattern[1] == 'a')	    pattern[1] = '9';
			else if (pattern[1] == '0') pattern[1] = 'f';
			else			    pattern[1] -= 1;

			if (pattern[1] == 'f')
			{
				if (pattern[0] == 'a') pattern[0] = '9';
				else		       pattern[0] -= 1;
			}
		}

		protocol->Write(String("Writing to ").Append(found));

		InStream	*in	  = new InStream(STREAM_FILE, found, IS_READ);
		OutStream	*out	  = new OutStream(STREAM_FILE, String(found).Append(".new"), OS_REPLACE);
		String		 idString = String("#FILENAME=").Append(cddbInfo.DiscIDToString());
		Bool		 written  = False;

		String	 inputFormat = String::SetInputFormat("ISO-8859-1");
		String	 outputFormat = String::SetOutputFormat("ISO-8859-1");

		while (in->GetPos() < in->Size())
		{
			String	 line = in->InputLine();

			out->OutputString(String(line).Append("\n"));

			if (line == idString)
			{
				out->OutputString(content.ConvertTo("UTF-8"));

				while (in->GetPos() < in->Size())
				{
					line = in->InputLine();

					if (line.StartsWith("#FILENAME=")) { out->OutputString(String(line).Append("\n")); break; }
				}

				written = True;
			}
		}

		if (!written)
		{
			out->OutputString(String(idString).Append("\n"));
			out->OutputString(content.ConvertTo("UTF-8"));
		}

		String::SetInputFormat(inputFormat);
		String::SetOutputFormat(outputFormat);

		delete in;
		delete out;

		File(found).Delete();
		File(String(found).Append(".new")).Move(found);
	}
	else						  // Unix style DB
	{
		protocol->Write("Found Unix style DB.");
		protocol->Write(String("Writing to ").Append(config->GetStringValue(Config::CategoryFreedbID, Config::FreedbDirectoryID, Config::FreedbDirectoryDefault)).Append(cddbInfo.category).Append(Directory::GetDirectoryDelimiter()).Append(cddbInfo.DiscIDToString()));

		OutStream	*out = new OutStream(STREAM_FILE, String(config->GetStringValue(Config::CategoryFreedbID, Config::FreedbDirectoryID, Config::FreedbDirectoryDefault)).Append(cddbInfo.category).Append(Directory::GetDirectoryDelimiter()).Append(cddbInfo.DiscIDToString()), OS_REPLACE);

		String	 outputFormat = String::SetOutputFormat("UTF-8");

		out->OutputString(content);

		String::SetOutputFormat(outputFormat);

		delete out;
	}

	protocol->Write("Leaving method.");

	return True;
}

Bool BonkEnc::CDDBLocal::CloseConnection()
{
	return True;
}
