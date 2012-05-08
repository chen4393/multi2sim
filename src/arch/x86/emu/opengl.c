/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <x86-emu.h>
#include <mem-system.h>

static char *err_x86_opengl_code =
	"\tAn invalid function code was generated by your application in a OpenGL system\n"
	"\tcall. Probably, this means that your application is using an incompatible\n"
	"\tversion of the Multi2Sim OpenGL runtime library ('libm2s-opengl'). Please\n"
	"\trecompile your application and try again.\n";


/* Debug */
int x86_opengl_debug_category;


/* List of OPENGL runtime calls */
enum x86_opengl_call_t
{
	x86_opengl_call_invalid = 0,
#define X86_OPENGL_DEFINE_CALL(name, code) x86_opengl_call_##name = code,
#include "opengl.dat"
#undef X86_OPENGL_DEFINE_CALL
	x86_opengl_call_count
};


/* List of OPENGL runtime call names */
char *x86_opengl_call_name[x86_opengl_call_count + 1] =
{
	NULL,
#define X86_OPENGL_DEFINE_CALL(name, code) #name,
#include "opengl.dat"
#undef X86_OPENGL_DEFINE_CALL
	NULL
};


/* Forward declarations of OPENGL runtime functions */
#define X86_OPENGL_DEFINE_CALL(name, code) static int x86_opengl_func_##name(void);
#include "opengl.dat"
#undef X86_OPENGL_DEFINE_CALL


/* List of OPENGL runtime functions */
typedef int (*x86_opengl_func_t)(void);
static x86_opengl_func_t x86_opengl_func_table[x86_opengl_call_count + 1] =
{
	NULL,
#define X86_OPENGL_DEFINE_CALL(name, code) x86_opengl_func_##name,
#include "opengl.dat"
#undef X86_OPENGL_DEFINE_CALL
	NULL
};


void x86_opengl_init(void)
{
}


void x86_opengl_done(void)
{
}


int x86_opengl_call(void)
{
	int code;
	int ret;

	/* Function code */
	code = x86_isa_regs->ebx;
	if (code <= x86_opengl_call_invalid || code >= x86_opengl_call_count)
		fatal("%s: invalid OpenGL function (code %d).\n%s",
			__FUNCTION__, code, err_x86_opengl_code);

	/* Debug */
	x86_opengl_debug("OpenGL runtime call '%s' (code %d)\n",
		x86_opengl_call_name[code], code);

	/* Call OPENGL function */
	assert(x86_opengl_func_table[code]);
	ret = x86_opengl_func_table[code]();

	/* Return value */
	return ret;
}




/*
 * OPENGL call #1 - init
 *
 * @param struct x86_opengl_version_t *version;
 *	Structure where the version of the OpenGL runtime implementation will be
 *	dumped. To succeed, the major version should match in the runtime
 *	library (guest) and runtime implementation (host), whereas the minor
 *	version should be equal or higher in the implementation (host).
 *
 *	Features should be added to the OpenGL runtime (guest and host) using the
 *	following rules:
 *	1)  If the guest library requires a new feature from the host
 *	    implementation, the feature is added to the host, and the minor
 *	    version is updated to the current Multi2Sim SVN revision both in
 *	    host and guest.
 *          All previous services provided by the host should remain available
 *          and backward-compatible. Executing a newer library on the older
 *          simulator will fail, but an older library on the newer simulator
 *          will succeed.
 *      2)  If a new feature is added that affects older services of the host
 *          implementation breaking backward compatibility, the major version is
 *          increased by 1 in the host and guest code.
 *          Executing a library with a different (lower or higher) major version
 *          than the host implementation will fail.
 *
 * @return
 *	The runtime implementation version is return in argument 'version'.
 *	The return value is always 0.
 */

#define X86_OPENGL_RUNTIME_VERSION_MAJOR	0
#define X86_OPENGL_RUNTIME_VERSION_MINOR	669

struct x86_opengl_version_t
{
	int major;
	int minor;
};

static int x86_opengl_func_init(void)
{
	unsigned int version_ptr;

	struct x86_opengl_version_t version;

	/* Arguments */
	version_ptr = x86_isa_regs->ecx;
	x86_opengl_debug("\tversion_ptr=0x%x\n", version_ptr);

	/* Return version */
	assert(sizeof(struct x86_opengl_version_t) == 8);
	version.major = X86_OPENGL_RUNTIME_VERSION_MAJOR;
	version.minor = X86_OPENGL_RUNTIME_VERSION_MINOR;
	mem_write(x86_isa_mem, version_ptr, 8, &version);
	x86_opengl_debug("\tOpenGL Runtime host implementation v. %d.%d\n", version.major, version.minor);

	/* Return success */
	return 0;
}

