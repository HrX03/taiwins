#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <linux/input.h>
#include <wayland-server.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-names.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "backend.h"
#include "desktop.h"
#include "taiwins.h"
#include "config.h"
#include "bindings.h"

//remove this two later

struct tw_compositor {
	struct weston_compositor *ec;

	struct taiwins_apply_bindings_listener add_binding;
	struct taiwins_config_component_listener config_component;
};


static FILE *logfile = NULL;

static int
tw_log(const char *format, va_list args)
{
	return vfprintf(logfile, format, args);
}

static void
taiwins_quit(struct weston_keyboard *keyboard,
	     const struct timespec *time,
	     uint32_t key, uint32_t option,
	     void *data)
{
	struct weston_compositor *compositor = data;
	fprintf(stderr, "quitting taiwins\n");
	struct wl_display *wl_display =
		compositor->wl_display;
	wl_display_terminate(wl_display);
	exit(1);
}

static bool
tw_compositor_add_bindings(struct tw_bindings *bindings, struct taiwins_config *c,
			struct taiwins_apply_bindings_listener *listener)
{
	struct tw_compositor *tc = container_of(listener, struct tw_compositor, add_binding);
	const struct tw_key_press *quit_press =
		taiwins_config_get_builtin_binding(c, TW_QUIT_BINDING)->keypress;
	tw_bindings_add_key(bindings, quit_press, taiwins_quit, 0, tc->ec);
	return true;
}


static void
tw_compositor_init(struct tw_compositor *tc, struct weston_compositor *ec,
		   struct taiwins_config *config)
{
	tc->ec = ec;
	struct xkb_rule_names sample_rules =  {
			.rules = NULL,
			.model = strdup("pc105"),
			.layout = strdup("us"),
			.options = strdup("ctrl:swap_lalt_lctl"),
		};
	weston_compositor_set_xkb_rule_names(ec, &sample_rules);

	wl_list_init(&tc->add_binding.link);
	tc->add_binding.apply = tw_compositor_add_bindings;
	taiwins_config_add_apply_bindings(config, &tc->add_binding);
}

int main(int argc, char *argv[], char *envp[])
{
	int error = 0;
	struct tw_compositor tc;
	const char *shellpath = (argc > 1) ? argv[1] : NULL;
	const char *launcherpath = (argc > 2) ? argv[2] : NULL;
	struct wl_display *display = wl_display_create();
	char config_file[100];

	logfile = fopen("/tmp/taiwins_log", "w");
	weston_log_set_handler(tw_log, tw_log);
	//quit if we already have a wayland server
	if (wl_display_add_socket(display, NULL) == -1)
		goto connect_err;

	struct weston_log_context *context =
		weston_log_ctx_compositor_create();
	struct weston_compositor *compositor =
		weston_compositor_create(display, context, NULL);
	//Hard code it here right now

	weston_log_set_handler(tw_log, tw_log);
	//get the configs now
	char *xdg_dir = getenv("XDG_CONFIG_HOME");
	if (!xdg_dir)
		xdg_dir = getenv("HOME");
	strcpy(config_file, xdg_dir);
	strcat(config_file, "/config.lua");
	struct taiwins_config *config =
		taiwins_config_create(compositor, tw_log);

	tw_compositor_init(&tc, compositor, config);
	tw_setup_backend(compositor, config);
	weston_compositor_wake(compositor);
	struct shell *sh = announce_shell(compositor, shellpath, config);
	announce_console(compositor, sh, launcherpath, config);
	announce_desktop(compositor, sh, config);

	error = !taiwins_run_config(config, config_file);
	if (error) {
		goto out;
	}
	compositor->kb_repeat_delay = 400;
	compositor->kb_repeat_rate = 40;

	wl_display_run(display);
out:
	taiwins_config_destroy(config);
	weston_compositor_tear_down(compositor);
	weston_log_ctx_compositor_destroy(compositor);

	weston_compositor_destroy(compositor);
	wl_display_destroy(display);
	return 0;
connect_err:
	wl_display_destroy(display);
	return -1;
}
