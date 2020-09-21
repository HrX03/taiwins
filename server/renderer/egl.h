/*
 * egl.h - taiwins EGL renderer interface
 *
 * Copyright (c) 2020 Xichen Zhou
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef TW_EGL_H
#define TW_EGL_H

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <assert.h>
#include <string.h>
#include <wayland-server.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct tw_egl_options {
	/** platform like EGL_PLATFORM_GBM_KHR */
	EGLenum platform;
	/** native display type like a wl_display from wayland */
	void *native_display;
	EGLint egl_surface_type;
	/** visual id represents the format the platform supports */
	EGLint visual_id;

	const EGLint *context_attribs;
	const uint32_t drm_formats;
	unsigned drm_formats_count;
};

struct tw_egl {
	EGLDisplay display;
	EGLenum platform;
	EGLConfig config;
	EGLContext context;
	bool query_buffer_age;

	struct {
		PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display;
		PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC create_platform_win;
		PFNEGLCREATEIMAGEKHRPROC create_image;
		PFNEGLDESTROYIMAGEKHRPROC destroy_image;
		PFNEGLQUERYWAYLANDBUFFERWL query_wl_buffer;
		PFNEGLBINDWAYLANDDISPLAYWL bind_wl_display;
		PFNEGLUNBINDWAYLANDDISPLAYWL unbind_wl_display;
		PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC swap_buffer_with_damage;
		PFNEGLQUERYDMABUFFORMATSEXTPROC query_dmabuf_formats;
		PFNEGLQUERYDMABUFMODIFIERSEXTPROC query_dmabuf_modifiers;
		PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC export_dmabuf_image_query;
		PFNEGLEXPORTDMABUFIMAGEMESAPROC export_dmabuf_image;
		PFNEGLDEBUGMESSAGECONTROLKHRPROC debug_message_control;
	} funcs;

	struct wl_display *wl_display;
};


bool
tw_egl_init(struct tw_egl *egl, struct tw_egl_options *opts);

void
tw_egl_fini(struct tw_egl *egl);

bool
tw_egl_check_gl_ext(struct tw_egl *egl, const char *ext);

bool
tw_egl_check_egl_ext(struct tw_egl *egl, const char *ext);

WL_EXPORT bool
tw_egl_make_current(struct tw_egl *egl, EGLSurface surface);

WL_EXPORT bool
tw_egl_unset_current(struct tw_egl *egl);

WL_EXPORT int
tw_egl_buffer_age(struct tw_egl *egl, EGLSurface surface);

#ifdef  __cplusplus
}
#endif


#endif /* EOF */
