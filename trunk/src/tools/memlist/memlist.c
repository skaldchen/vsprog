/***************************************************************************
 *   Copyright (C) 2009 by Simon Qian <SimonQian@SimonQian.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "app_type.h"
#include "app_log.h"
#include "app_err.h"

#include "memlist.h"

#define MEMLIST_AdjustAddr(addr, page_size)		\
						((addr) / (page_size) * (page_size))
#define MEMLIST_AdjustLen(len, page_size)		\
						(((len) + (page_size) - 1) / (page_size) * (page_size))

static RESULT MEMLIST_Merge(memlist *ml, uint32_t addr, uint32_t len, 
							uint32_t page_size)
{
	while (ml != NULL)
	{
		if ((addr >= ml->addr) && ((ml->addr + ml->len) >= addr))
		{
			ml->len = MEMLIST_AdjustLen(len + addr - ml->addr, page_size);
			return ERROR_OK;
		}
		ml = MEMLIST_GetNext(ml);
	}
	
	return ERROR_FAIL;
}

static void MEMLIST_InsertLast(memlist *ml, memlist *newitem)
{
	while (MEMLIST_GetNext(ml) != NULL)
	{
		ml = MEMLIST_GetNext(ml);
	}
	
	ml->list.next = &newitem->list;
}

RESULT MEMLIST_Add(memlist **ml, uint32_t addr, uint32_t len, uint32_t page_size)
{
	memlist *newitem = NULL;
	
	if (NULL == *ml)
	{
		*ml = (memlist*)malloc(sizeof(memlist));
		if (NULL == *ml)
		{
			LOG_ERROR(_GETTEXT(ERRMSG_NOT_ENOUGH_MEMORY));
			return ERROR_FAIL;
		}
		
		(*ml)->addr = MEMLIST_AdjustAddr(addr, page_size);
		(*ml)->len = MEMLIST_AdjustLen(len, page_size);
		sllist_init_node((*ml)->list);
	}
	else
	{
		if (ERROR_OK != MEMLIST_Merge(*ml, addr, len, page_size))
		{
			newitem = (memlist*)malloc(sizeof(memlist));
			if (NULL == newitem)
			{
				LOG_ERROR(_GETTEXT(ERRMSG_NOT_ENOUGH_MEMORY));
				return ERROR_FAIL;
			}
			
			newitem->addr = MEMLIST_AdjustAddr(addr, page_size);
			newitem->len = MEMLIST_AdjustLen(len, page_size);
			sllist_init_node(newitem->list);
			MEMLIST_InsertLast(*ml, newitem);
		}
	}
	
	return ERROR_OK;
}

void MEMLIST_Free(memlist **ml)
{
	memlist *tmp1, *tmp2;
	
	tmp1 = *ml;
	while (tmp1 != NULL)
	{
		tmp2 = tmp1;
		tmp1 = MEMLIST_GetNext(tmp1);
		sllist_init_node(tmp2->list);
		free(tmp2);
	}
	tmp1 = tmp2 = NULL;
	*ml = NULL;
}

