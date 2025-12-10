// test_pi.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

double read_cpu_temp(void) {
    const char *path = "/sys/class/thermal/thermal_zone0/temp";
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1.0;  // could not read
    }

    long temp_milli = 0;
    if (fscanf(fp, "%ld", &temp_milli) != 1) {
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    return temp_milli / 1000.0;  // convert to °C
}

int main(void) {
    printf("=== Raspberry Pi C Test Program ===\n\n");

    // Hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("Hostname        : %s\n", hostname);
    } else {
        perror("gethostname");
    }

    // System info
    struct utsname sysinfo;
    if (uname(&sysinfo) == 0) {
        printf("System          : %s\n", sysinfo.sysname);
        printf("Node name       : %s\n", sysinfo.nodename);
        printf("Release         : %s\n", sysinfo.release);
        printf("Version         : %s\n", sysinfo.version);
        printf("Machine         : %s\n", sysinfo.machine);
    } else {
        perror("uname");
    }

    // CPU temperature
    double temp = read_cpu_temp();
    if (temp > 0.0) {
        printf("CPU temperature : %.1f °C\n", temp);
    } else {
        printf("CPU temperature : (could not read)\n");
    }

    printf("\nIf you see this output, the Pi booted and your C program runs correctly.\n");
    return 0;
}
