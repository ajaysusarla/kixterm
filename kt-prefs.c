/*
 * kt-prefs.c
 *
 * Part of the kixterm project.
 *
 * Copyright © 2014 Partha Susarla <ajaysusarla@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kt-prefs.h"
#include "kt-color-scheme.h"

struct _KtPrefsPriv {
        gboolean ignore;
};

G_DEFINE_TYPE(KtPrefs, kt_prefs, G_TYPE_OBJECT);

/* Private methods */

/* Class methods */
static void kt_prefs_finalize(GObject *object)
{

        G_OBJECT_CLASS(kt_prefs_parent_class)->finalize(object);
}

static void kt_prefs_class_init(KtPrefsClass *klass)
{
        GObjectClass *oclass = G_OBJECT_CLASS(klass);

        oclass->finalize = kt_prefs_finalize;

        g_type_class_add_private(klass, sizeof(KtPrefsPriv));
}

static void kt_prefs_init(KtPrefs *prefs)
{
        prefs->priv = G_TYPE_INSTANCE_GET_PRIVATE(prefs,
                                                  KT_PREFS_TYPE,
                                                  KtPrefsPriv);

        prefs->title = "kixterm";
        prefs->xpos = -1;
        prefs->ypos = -1;
        prefs->rows = 24; /* 80x24 */
        prefs->cols = 80;
        prefs->sb_width = 6;
        prefs->bd_width = 1;

        prefs->font_name = "Monospace";
        prefs->font_size = 12;
}

/* Public methods */
KtPrefs *kt_prefs_new(void)
{
        KtPrefs *prefs = NULL;

        prefs = g_object_new(KT_PREFS_TYPE, NULL);

        prefs->fg_color = linux_colors[7];
        prefs->bg_color = linux_colors[0];
        prefs->vb_color = reserved_colors[0];

        return prefs;
}
