#include <QCoreApplication>
#include <QDebug>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <libdrm/amdgpu_drm.h>
#include <libdrm/amdgpu.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /*uint64_t data;
    uint sensor;
    struct drm_amdgpu_info buffer = {};
    buffer.query = AMDGPU_INFO_SENSOR;
    buffer.return_pointer = data;
    buffer.return_size = 32;
    buffer.sensor_info.type = sensor;*/

    int fd = 0;
    fd = open("/dev/dri/renderD128", O_RDONLY);
    qDebug() << fd;

    char driver[64];
    drm_version_t ver;
    ver.name = driver;
    ver.name_len = 64;
    ioctl(fd, DRM_IOCTL_VERSION, &ver);
    qDebug() << ver.name;

    uint32_t major = 0;
    uint32_t minor = 0;
    amdgpu_device_handle handle;
    int ret = amdgpu_device_initialize(fd, &major, &minor, &handle);
    qDebug() << ret << major;

    int reading = 0;
    uint size = 64;
    ret = amdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_GPU_TEMP, size, &reading);
    qDebug() << "temp" << reading << ret;

    ret = amdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_VDDGFX, size, &reading);
    qDebug() << "voltage" << reading << ret;

    ret = amdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_GFX_SCLK, size, &reading);
    qDebug() << "coreclk" << reading << ret;

    ret = amdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_GFX_MCLK, size, &reading);
    qDebug() << "memclk" << reading << ret;

    /*char *name;
    char *busid;
    int ret = drmOpen(name, busid);
    qDebug() << ret;

    uint32_t major;
    uint32_t minor;
    amdgpu_device_handle handle;
    ret = amdgpu_device_initialize(fd, &major, &minor, &handle);
    qDebug() << ret;
    int ret = drmAvailable();
    qDebug() << ret;

    char *name;
    char *busid;
    ret = drmOpen(name, busid);
    qDebug() << ret;

    qDebug() << busid;
    uint32_t major;
    uint32_t minor;
    amdgpu_device_handle handle;*/

    return a.exec();
}
