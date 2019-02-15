// clang-format off
#include <stdio.h>

enum class TTRunType {
	TopLevelProcess,
	UnderMaster,
	UnderDebugger,
};

static TTRunType TTGetRunType()
{
#ifdef _WIN32
#ifdef _UNICODE
	//OutputDebugString(GetCommandLine());
	//printf( "IsRunningUnderMaster? %d\n", wcsstr( GetCommandLine(), L" test =" ) != nullptr );
	if (wcsstr(GetCommandLine(), L" test =") != nullptr)
		return TTRunType::UnderMaster;
	else if (wcsstr(GetCommandLine(), L" test :") != nullptr)
		return TTRunType::UnderDebugger;
	else
		return TTRunType::TopLevelProcess;
#else
	if (strstr(GetCommandLine(), " test =") != nullptr)
		return TTRunType::UnderMaster;
	else if (strstr(GetCommandLine(), " test :") != nullptr)
		return TTRunType::UnderDebugger;
	else
		return TTRunType::TopLevelProcess;

#endif
#else
	TTRunType res;
	FILE* f = fopen("/proc/self/cmdline", "r");
	if (f != nullptr)
	{
		char buf[1024];
		int len = fread(buf, 1, 1023, f);
		fclose(f);
		buf[len] = 0;
		for (int i = 0; i < len; i++) {
			// /proc/*/cmdline replaces whitespace with 0
			if (buf[i] == 0)
				buf[i] = 32;
		}
		if (strstr(buf, " test =") != nullptr)
			res = TTRunType::UnderMaster;
		else if (strstr(buf, " test :") != nullptr)
			res =TTRunType::UnderDebugger;
		else
			res = TTRunType::TopLevelProcess;
	}
	return res;
#endif
}

TT_UNIVERSAL_FUNC bool TTIsRunningUnderMaster()
{
	return TTGetRunType() == TTRunType::UnderMaster;
}

TT_UNIVERSAL_FUNC bool TTIsRunningUnderDebugger()
{
	return TTGetRunType() == TTRunType::UnderDebugger;
}

TT_UNIVERSAL_FUNC void TTAssertFailed(const char* exp, const char* filename, int line_number, bool die)
{
	printf("Test Failed:\nExpression: %s\nFile: %s\nLine: %d", exp, filename, line_number);
	fflush(stdout);
	fflush(stderr);

	if (TTIsRunningUnderMaster())
	{
#ifdef _WIN32
		// Use TerminateProcess instead of exit(), because we don't want any C++ cleanup, or CRT cleanup code to run.
		// Such "at-exit" functions are prone to popping up message boxes about resources that haven't been cleaned up, but
		// at this stage, that is merely noise.
		TerminateProcess(GetCurrentProcess(), 1);
#else
		_exit(1);
#endif
	}
	else
	{
#ifdef _WIN32
		__debugbreak();
#else
		__builtin_trap();
#endif
	}
}
