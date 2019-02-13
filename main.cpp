#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
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

    // Read fan speed in % and mode (1 = automatic, 2 = manual)
    QString monpath = "/sys/class/drm/card0/device/hwmon";

    // Regexp for the hwmon folder since the index can change
    QDir mondir(monpath);
    qDebug() << mondir.entryList() << "mondir";
    QStringList list = mondir.entryList();
    for (int i=0; i<list.size(); i++) {
        if (list[i].contains("hwmon")) {
            qDebug() << list[i];
            mondir.setPath(monpath+"/"+list[i]);
        }
    }
    qDebug() << mondir.entryList();

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
    int valuetype = 0;
    QVector <int> memvolts, corevolts, core, mem;
    int maxvolt = 0;
    int minvolt = 0;
    int minmemclk = 0;
    int maxmemclk = 0;
    int maxcoreclk = 0;
    int mincoreclk = 0;
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


            valuetype++;
            QString capline = nummatch.captured();
            int num = capline.toInt();
            QByteArray arr = line.toLocal8Bit();
            linechar = arr.data();
            QByteArray caparr = nummatch.captured().toLocal8Bit();
            capnum = caparr.data();
            if (type == 1) {
                //printf("Core values: %s %s\n", capnum, linechar);
                if (valuetype == 0) {
                    core.append(num);
                } else {
                    corevolts.append(num);
                }
            }
            if (type == 2) {
                //printf("Memory values: %s %s\n", capnum, linechar);
                if (valuetype == 0) {
                    mem.append(num);
                } else {
                    memvolts.append(num);
                }
            }
            if (type == 3) {
                //printf("Range values: %s %s\n", capnum, linechar);
                if (line.contains("sclk", Qt::CaseInsensitive)) {
                    if (valuetype == 0) mincoreclk = num;
                    else maxcoreclk = num;
                }
                if (line.contains("mclk", Qt::CaseInsensitive)) {
                    if (valuetype == 0) minmemclk = num;
                    else maxmemclk = num;
                }
                if (line.contains("vddc", Qt::CaseInsensitive)) {
                    if (valuetype == 0) minvolt = num;
                    else maxvolt = num;
                }
            }

        }
        breakcount++;
        valuetype = 0;
    }

    for (int i=0; i<core.size(); i++) {
        printf("Core clock pstate %d: %d %d\n", i, core[i], corevolts[i]);
    }
    for (int i=0; i<mem.size(); i++) {
        printf("Mem clock pstate %d: %d %d\n", i, mem[i], memvolts[i]);
    }
    printf("Voltage limits: %d-%d\n", minvolt, maxvolt);
    printf("Core clock limits: %d-%d\n", mincoreclk, maxcoreclk);
    printf("Mem clock limits: %d-%d\n", minmemclk, maxmemclk);

    return a.exec();
}
