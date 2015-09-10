 /* fre:ac - free audio converter
  * Copyright (C) 2001-2015 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#ifndef H_FREAC_CDDB_QUERY
#define H_FREAC_CDDB_QUERY

#include <smooth.h>

using namespace smooth;
using namespace smooth::GUI;

#include <cddb/cddb.h>
#include <cddb/cddbinfo.h>

namespace BonkEnc
{
	class cddbQueryDlg : public Dialogs::Dialog
	{
		private:
			Window		*mainWnd;
			Titlebar	*mainWnd_titlebar;

			Text		*text_status;
			Progressbar	*prog_status;
			Button		*btn_cancel;

			Config		*currentConfig;

			CDDBInfo	 cddbInfo;
			String		 queryString;

			Bool		 allowAddToBatch;

			Threads::Thread	*queryThread;
			Bool		 stopQueryThread;

			Void		 Cancel();
			Int		 QueryThread();

			Bool		 QueryCDDB(CDDB &, Bool);
		public:
					 cddbQueryDlg();
					~cddbQueryDlg();

			const Error	&ShowDialog();

			Bool		 SetQueryString(const String &);

			const CDDBInfo	&QueryCDDB(Bool);
	};
};

#endif
