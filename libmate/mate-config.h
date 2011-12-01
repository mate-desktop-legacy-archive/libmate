/*
 * Copyright (C) 1993, 1994, 1997, 1998, 1999, 2000 Free Software Foundation
 * All rights reserved.
 *
 * This file is part of the Mate Library.
 *
 * The Mate Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Mate Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Mate Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
/*
  @NOTATION@
 */

#ifndef MATE_CONFIG_H
#define MATE_CONFIG_H

#ifndef MATE_DISABLE_DEPRECATED

#include <glib.h>

/* Make sure we have the directory getters */
#include <libmate/mate-init.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Prototypes for the profile management functions */

/*
 * DOC: mate configuration routines.
 *
 * All of the routines receive a pathname, the pathname has the following
 * form:
 *
 *      /filename/section/key[=default]
 *
 * This format reprensents: a filename relative to the Mate config
 * directory called filename (ie, ~/.mate/filename), in that file there
 * is a section called [section] and key is the left handed side of the
 * values.
 *
 * If default is provided, it cane be used to return a default value
 * if none is specified on the config file.
 *
 * Examples:
 *
 * /gmix/Balance/Ratio=0.5
 * /filemanager/Panel Display/html=1
 *
 * If the pathname starts with '=', then instead of being a ~/.mate relative
 * file, it is an abolute pathname, example:
 *
 * =/home/miguel/.mc.ini=/Left Panel/reverse=1
 *
 * This reprensents the config file: /home/miguel/.mc.ini, section [Left Panel],
 * variable reverse.
 */

/* These functions look for the config option named in PATH.  If the
   option does not exist, and a default is specified in PATH, then the
   default will be returned.  In all cases, *DEF is set to 1 if the
   default was return, 0 otherwise.  If DEF is NULL then it will not
   be set.  */

/*use the wrappers below*/
char* mate_config_get_string_with_default_(const char* path, gboolean* def, gboolean priv);
char* mate_config_get_translated_string_with_default_(const char* path, gboolean* def, gboolean priv);
gint  mate_config_get_int_with_default_(const char *path, gboolean *def, gboolean priv);
gdouble  mate_config_get_float_with_default_(const char* path, gboolean* def, gboolean priv);
gboolean mate_config_get_bool_with_default_(const char* path, gboolean* def, gboolean priv);
void mate_config_get_vector_with_default_(const char* path, gint* argcp, char*** argvp, gboolean* def, gboolean priv);

/*these just call the above functions, but devide them into two groups,
  in the future these may be different functions, so use these defines*/
/*normal functions*/
#define mate_config_get_string_with_default(path, def) \
	(mate_config_get_string_with_default_((path), (def), FALSE))
#define mate_config_get_translated_string_with_default(path, def) \
	(mate_config_get_translated_string_with_default_((path), (def), FALSE))
#define mate_config_get_int_with_default(path, def) \
	(mate_config_get_int_with_default_((path), (def),FALSE))
#define mate_config_get_float_with_default(path, def) \
	(mate_config_get_float_with_default_((path), (def), FALSE))
#define mate_config_get_bool_with_default(path, def) \
	(mate_config_get_bool_with_default_((path), (def), FALSE))
#define mate_config_get_vector_with_default(path, argcp, argvp, def) \
	(mate_config_get_vector_with_default_((path), (argcp), (argvp), (def), FALSE))

/*private functions*/
#define mate_config_private_get_string_with_default(path, def) \
	(mate_config_get_string_with_default_((path), (def), TRUE))
#define mate_config_private_get_translated_string_with_default(path, def) \
	(mate_config_get_translated_string_with_default_((path), (def), TRUE))
#define mate_config_private_get_int_with_default(path, def) \
	(mate_config_get_int_with_default_((path), (def), TRUE))
#define mate_config_private_get_float_with_default(path, def) \
	(mate_config_get_float_with_default_((path), (def), TRUE))
#define mate_config_private_get_bool_with_default(path, def) \
	(mate_config_get_bool_with_default_((path), (def), TRUE))
#define mate_config_private_get_vector_with_default(path, argcp, argvp, def) \
	(mate_config_get_vector_with_default_ ((path), (argcp), (argvp), (def), TRUE))

/* Convenience wrappers for the case when you don't care if you see
   the default.  */
/*normal functions*/
#define mate_config_get_string(path) \
	(mate_config_get_string_with_default_((path), NULL, FALSE))
#define mate_config_get_translated_string(path) \
	(mate_config_get_translated_string_with_default_((path), NULL, FALSE))
#define mate_config_get_int(path) \
	(mate_config_get_int_with_default_((path), NULL, FALSE))
#define mate_config_get_float(path) \
	(mate_config_get_float_with_default_((path), NULL, FALSE))
#define mate_config_get_bool(path) \
	(mate_config_get_bool_with_default_((path), NULL, FALSE))
#define mate_config_get_vector(path, argcp, argvp) \
	(mate_config_get_vector_with_default_((path), (argcp), (argvp), NULL, FALSE))

/*private functions*/
#define mate_config_private_get_string(path) \
	(mate_config_get_string_with_default_((path), NULL, TRUE))
#define mate_config_private_get_translated_string(path) \
	(mate_config_get_translated_string_with_default_((path), NULL, TRUE))
#define mate_config_private_get_int(path) \
	(mate_config_get_int_with_default_((path), NULL, TRUE))
#define mate_config_private_get_float(path) \
	(mate_config_get_float_with_default_((path), NULL, TRUE))
#define mate_config_private_get_bool(path) \
	(mate_config_get_bool_with_default_((path), NULL, TRUE))
#define mate_config_private_get_vector(path, argcp, argvp) \
	(mate_config_get_vector_with_default_((path), (argcp), (argvp), NULL, TRUE))

/* Set a config variable.  Use the warppers below*/
void mate_config_set_string_(const char* path, const char* value, gboolean priv);
void mate_config_set_translated_string_ (const char* path, const char* value, gboolean priv);
void mate_config_set_int_(const char* path, int value, gboolean priv);
void mate_config_set_float_(const char* path, gdouble value, gboolean priv);
void mate_config_set_bool_(const char* path, gboolean value, gboolean priv);
void mate_config_set_vector_(const char* path, int argc, const char* const argv[], gboolean priv);


/* normal functions */
#define mate_config_set_string(path,new_value) \
	(mate_config_set_string_((path),(new_value),FALSE))
#define mate_config_set_translated_string(path,value) \
	(mate_config_set_translated_string_((path),(value),FALSE))
#define mate_config_set_int(path,new_value) \
	(mate_config_set_int_((path),(new_value),FALSE))
#define mate_config_set_float(path,new_value) \
	(mate_config_set_float_((path),(new_value),FALSE))
#define mate_config_set_bool(path,new_value) \
	(mate_config_set_bool_((path),(new_value),FALSE))
#define mate_config_set_vector(path,argc,argv) \
	(mate_config_set_vector_((path),(argc),(argv),FALSE))

/* private functions */
#define mate_config_private_set_string(path,new_value) \
	(mate_config_set_string_((path),(new_value),TRUE))
#define mate_config_private_set_translated_string(path,new_value) \
	(mate_config_set_translated_string_((path),(new_value),TRUE))
#define mate_config_private_set_int(path,new_value) \
	(mate_config_set_int_((path),(new_value),TRUE))
#define mate_config_private_set_float(path,new_value) \
	(mate_config_set_float_((path),(new_value),TRUE))
#define mate_config_private_set_bool(path,new_value) \
	(mate_config_set_bool_((path),(new_value),TRUE))
#define mate_config_private_set_vector(path,argc,argv) \
	(mate_config_set_vector_((path),(argc),(argv),TRUE))

/* Returns true if /path/section is defined */
gboolean  mate_config_has_section_(const char* path, gboolean priv);
#define mate_config_has_section(path) (mate_config_has_section_((path), FALSE))
#define mate_config_private_has_section(path) (mate_config_has_section_((path), TRUE))

/* Returns a pointer for iterating on /file/section contents */
void* mate_config_init_iterator_(const char* path, gboolean priv);
#define mate_config_init_iterator(path) (mate_config_init_iterator_((path),FALSE))
#define mate_config_private_init_iterator(path) (mate_config_init_iterator_((path),TRUE))

/* Returns a pointer for iterating on /file contents */
void* mate_config_init_iterator_sections_(const char* path, gboolean priv);
#define mate_config_init_iterator_sections(path) (mate_config_init_iterator_sections_((path), FALSE))
#define mate_config_private_init_iterator_sections(path) (mate_config_init_iterator_sections_((path), TRUE))

/* Get next key and value value from a section */
void* mate_config_iterator_next(void* iterator_handle, char** key, char** value);

void mate_config_drop_all(void);

gboolean mate_config_sync(void);

/* sync's data for one file only */
gboolean mate_config_sync_file_(char* path, gboolean priv);
#define mate_config_sync_file(path) (mate_config_sync_file_((path), FALSE))
#define mate_config_private_sync_file(path) (mate_config_sync_file_((path), TRUE))

/* This routine drops the information about /file, meaning changes
   done to this file will be dropped, it will no delete the file */
void mate_config_drop_file_(const char* path, gboolean priv);
#define mate_config_drop_file(path) (mate_config_drop_file_((path), FALSE))
#define mate_config_private_drop_file(path) (mate_config_drop_file_((path), TRUE))

/* This routine actually removes /file on sync (not right away, you
   can still save it by dropping it)*/
void mate_config_clean_file_(const char* path, gboolean priv);
#define mate_config_clean_file(path) (mate_config_clean_file_((path), FALSE))
#define mate_config_private_clean_file(path) (mate_config_clean_file_((path), TRUE))

/* This routine drops all of the information related to /file/section
   this will actually remove the section */
void mate_config_clean_section_(const char* path, gboolean priv);
#define mate_config_clean_section(path) (mate_config_clean_section_((path),FALSE))
#define mate_config_private_clean_section(path) (mate_config_clean_section_((path),TRUE))

/* Drops the information for a specific key, this will actually remove
   the key */
void mate_config_clean_key_(const char* path, gboolean priv);
#define mate_config_clean_key(path) (mate_config_clean_key_((path), FALSE))
#define mate_config_private_clean_key(path) (mate_config_clean_key_((path), TRUE))

/* returns the true filename of the config file */
#define mate_config_get_real_path(path) (g_build_filename(mate_user_dir_get(), (path), NULL))
#define mate_config_private_get_real_path(path) (g_build_filename(mate_user_private_dir_get(), (path), NULL))

/* Set an active prefix and remove an active prefix */
void mate_config_push_prefix(const char* path);
void mate_config_pop_prefix(void);

/*
 * Internal routines that we export
 * Used to go from string->vector and from vector->string
 */
void mate_config_make_vector(const char* string, int* argcp, char*** argvp);
char* mate_config_assemble_vector(int argc, const char* const argv[]);

/* these two are absolutely obscolete and should not be used */
void mate_config_set_set_handler(void (*func)(void*), void *data);
void mate_config_set_sync_handler(void (*func)(void*), void* data);

#ifdef __cplusplus
}
#endif

#endif

#endif /* MATE_DISABLE_DEPRECATED */

