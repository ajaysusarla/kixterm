/*
 * kt-app.c
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

#include "kt-app.h"

G_DEFINE_TYPE(KixtermApp, kixterm_app, G_TYPE_OBJECT);

/* Private methods */

/* Class methods */
static void kixterm_app_finalize(GObject *object)
{

        G_OBJECT_CLASS(kixterm_app_parent_class)->finalize(object);
}

static void kixterm_app_class_init(KixtermAppClass *klass)
{
        GObjectClass *oclass = G_OBJECT_CLASS(klass);

        oclass->finalize = kixterm_app_finalize;
}

static void kixterm_app_init(KixtermApp *object)
{
}


/* Public methods */
KixtermApp *kixterm_app_new(void)
{
        KixtermApp *app = NULL;

        app = g_object_new(KIXTERM_APP_TYPE, NULL);

        return app;
}
