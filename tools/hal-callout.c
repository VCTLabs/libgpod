#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>
#define __USE_BSD /* for mkdtemp */
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <itdb.h>
#include <itdb_device.h>

extern char *read_sysinfo_extended (const char *device);

static char *mount_ipod (const char *dev_path)
{
        char *filename;
        char *tmpname;
        const char *fstype;
        int result;

        fstype = g_getenv ("HAL_PROP_VOLUME_FSTYPE");
        if (fstype == NULL) {
                return NULL;
        }
        filename = g_build_filename (G_DIR_SEPARATOR_S, "tmp", "ipodXXXXXX",
                                     NULL);
        if (filename == NULL) {
                return NULL;
        }
        tmpname = mkdtemp (filename);
        if (tmpname == NULL) {
                g_free (filename);
                return NULL;
        }
        g_assert (tmpname == filename);
        result = mount (dev_path, tmpname, fstype, 0, NULL);
        if (result != 0) {
                g_rmdir (filename);
                g_free (filename);
                return NULL;
        }

        return tmpname;
}

static gboolean write_sysinfo_extended (const char *mountpoint, 
                                        const char *data)
{
        char *filename;
        char *devdirpath;
        gboolean result;

        devdirpath = itdb_get_device_dir (mountpoint);
        if (devdirpath == NULL) {
                return FALSE;
        }
        filename = itdb_get_path (devdirpath, "SysInfoExtended");
        g_free (devdirpath);
        if (filename == NULL) {
                return FALSE;
        }

        result = g_file_set_contents (filename, data, -1, NULL);
        g_free (filename);

        return result;
}

int main (int argc, char **argv)
{
        char *ipod_mountpoint;
        char *xml;

        xml = read_sysinfo_extended (g_getenv ("HAL_PROP_BLOCK_DEVICE"));
        if (xml == NULL) {
                return -1;
        }
        ipod_mountpoint = mount_ipod (g_getenv ("HAL_PROP_BLOCK_DEVICE"));
        if (ipod_mountpoint == NULL) {
                g_free (xml);
                return -1;
        }
        write_sysinfo_extended (ipod_mountpoint, xml); 

        umount (ipod_mountpoint);
        g_rmdir (ipod_mountpoint);
        g_free (ipod_mountpoint);
        g_free (xml);

        return 0;
}
