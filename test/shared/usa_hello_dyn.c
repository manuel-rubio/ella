#include <dlfcn.h>
#include <stdio.h>

int main() {
	void *handle = dlopen("libhello.so.1", RTLD_LAZY);
	if (handle == 0) {
		printf ("%s", dlerror());
		return(-1);
	}
	void (*test)() = dlsym (handle, "saluda");
	if (test == 0) {
		printf ("%s", dlerror());
		return(-1);
	}
	(*test) ();
	dlclose(handle);
	return 0;
}

