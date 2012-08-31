/* -*- Mode: C; c-set-style: gnu indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* mate-url.c
 * Copyright (C) 1998, James Henstridge <james@daa.com.au>
 * Copyright (C) 1999, 2000 Red Hat, Inc.
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
/*
  @NOTATION@
 */

#include <config.h>
#include <string.h>
#include <glib.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <glib/gi18n-lib.h>

#ifndef G_OS_WIN32
	#include <gio/gio.h>
#else
	#include <windows.h>
#endif

#include "mate-exec.h"
#include "mate-util.h"
#include "mate-init.h"

#include "mate-url.h"

/**
 * mate_url_show:
 * @url: The url or path to display. The path can be relative to the current working
 * directory or the user's home directory. This function will convert it into a fully
 * qualified url using the mate_url_get_from_input function.
 * @error: Used to store any errors that result from trying to display the @url.
 *
 * Once the input has been converted into a fully qualified url this function
 * calls g_app_info_launch_default_for_uri. All errors comes from the %MATE_URL_ERROR% domain.
 *
 * Returns: %TRUE if everything went fine, %FALSE otherwise (in which case
 * @error will contain the actual error).
 */
gboolean mate_url_show(const char* url, GError** error)
{
	#ifndef G_OS_WIN32

		g_return_val_if_fail(url != NULL, FALSE);

		return g_app_info_launch_default_for_uri (url, NULL, error);

	#else

		/* FIXME: Just call ShellExecute... Good enough? */
		if ((int) ShellExecute(HWND_DESKTOP, "open", url, NULL, NULL, SW_SHOWNORMAL) <= 32)
		{
			g_set_error(error, MATE_URL_ERROR, MATE_URL_ERROR_LAUNCH, _("There was an error launching the default action command associated with this location."));
			return FALSE;
		}

		return TRUE;

	#endif
}

/**
 * mate_url_error_quark
 *
 * Returns: A quark representing mate-url module errors.
 */
GQuark mate_url_error_quark(void)
{
	static GQuark error_quark = 0;

	if (error_quark == 0)
	{
		error_quark = g_quark_from_static_string("mate-url-error-quark");
	}

	return error_quark;
}
