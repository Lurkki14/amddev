#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QRegularExpressionMatch>
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
    //qDebug() << fd;
    printf("%d\n", fd);

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
    //qDebug() << ret << major;
    printf("%d %d\n", major, ret);

    int reading = 0;
    uint size = sizeof (int);
    ret = amdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_GPU_TEMP, size, &reading);
    //qDebug() << "temp" << reading << ret;
    printf("temp: %d\n", reading);

    ret = amdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_VDDGFX, size, &reading);
    //qDebug() << "voltage" << reading << ret;
    printf("voltage: %d\n", reading);

    ret = amdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_GFX_SCLK, size, &reading);
    //qDebug() << "coreclk" << reading << ret;
    printf("coreclk: %d\n", reading);

    ret = amdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_GFX_MCLK, size, &reading);
    //qDebug() << "memclk" << reading << ret;
    printf("memclk: %d\n", reading);

    ret = amdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_GPU_AVG_POWER, size, &reading);
    //qDebug() << "powerdraw" << reading << ret;
    printf("powerdraw: %d\n", reading);

    ret = amdgpu_query_sensor_info(handle, AMDGPU_INFO_SENSOR_GPU_LOAD, size, &reading);
    //qDebug() << "coreutil" << reading << ret;
    printf("gpuutil: %d\n", reading);

    const char *name = amdgpu_get_marketing_name(handle);
    //qDebug() << name;
    printf("name: %s\n", name);


    amdgpu_gpu_info info;
    ret = amdgpu_query_gpu_info(handle, &info);
    // These are the values for the highest pstate
    printf("max memclk: %d\n", info.max_memory_clk);
    printf("max coreclk: %d\n", info.max_engine_clk);


    /*drm_amdgpu_info_vce_clock_table table;
    printf("number of clock table entries: %d\n", table.num_valid_entries);
    for (int i=0; i<table.num_valid_entries; i++) {
        printf("entry %d: %d %d\n", i, table.entries[i].sclk, table.entries[i].mclk);
    }*/

    // Read the pp_od_clk_voltage for the current GPU
    QString path = "/sys/class/drm/card0/device/pp_od_clk_voltage";
    //QString path = "/etc/X11/xorg.conf.d/20-nvidia.conf.bak";
    QFile file(path);
    bool retb = file.open(QFile::ReadOnly | QFile::Text);
    if (retb) printf("File opened successfully\n");
    else printf("Failed to open file\n");
    QTextStream str(&file);
    QString line;
    QRegularExpression numexp("\\d+\\d");
    char *linechar;
    int breakcount = 0;
    char *capnum;
    int type = 0;
    while (!str.atEnd() && breakcount < 50) {
        line = str.readLine();
        if (line.contains("OD_SCLK")) type = 1;
        if (line.contains("OD_MCLK")) type = 2;
        if (line.contains("OD_RANGE")) type = 3;

        QRegularExpressionMatchIterator i = numexp.globalMatch(line);
        while (i.hasNext()) {
            QRegularExpressionMatch nummatch = i.next();
            /*switch (type) {
                case 1:
                    qDebug() << nummatch.captured() << line <<"core clock values";
                    break;
                case 2:
                    QRegularExpressionMatch nummatch = i.next();
                    qDebug() << nummatch.captured() << line <<"mem clock values";
                    break;
            }*/
            QByteArray arr = line.toLocal8Bit();
            linechar = arr.data();
            QByteArray caparr = nummatch.captured().toLocal8Bit();
            capnum = caparr.data();
            if (type == 1) printf("Core values: %s %s\n", capnum, linechar);
            if (type == 2) printf("Memory values: %s %s\n", capnum, linechar);
            if (type == 3) printf("Range values: %s %s\n", capnum, linechar);
        }
        breakcount++;
    }

    return a.exec();
}
