#ifndef TESTDBHELPER_H
#define TESTDBHELPER_H

#include "BaseDbHelper.h"
#include <QJsonObject>
#include <QDateTime>

// 配置参数结构体
struct ConfigParams {
    // 所有Python参数（对应数据库表字段）
    int user_id = -1;
    int init_seed = 1;
    int campaign_run = 2000;
    int tot_runs = 20;
    int max_loop_number = 1;
    int max_iters = 5000;
    QString system_name = "nomoto_ship";
    double x_star_1 = 30.0;
    double x_star_2 = 0.0;

    int N = 500;
    int N_max = 1000;
    bool sliding_window = true;
    double learning_rate = 0.01;
    double learning_rate_c = 0.1;
    bool use_scheduler = true;
    int sched_T = 300;
    int print_interval = 200;

    int n_input = 2;
    double beta_sfpl = 2;
    bool clipping_V = true;
    QString size_layers = "[10,10,1]";
    QString lyap_activations = "[\"pow2\",\"linear\",\"linear\"]";
    QString lyap_bias = "[false,false,false]";

    bool use_lin_ctr = false;
    bool lin_contr_bias = false;
    bool control_initialised = false;
    QString init_control = "[[-23.58639732, -5.31421063]]";
    QString size_ctrl_layers = "[50,1]";
    QString ctrl_bias = "[true,false]";
    QString ctrl_activations = "[\"tanh\",\"linear\"]";
    bool use_saturation = true;
    QString ctrl_sat = "[35.0]";

    double gamma_underbar = 0.1;
    double gamma_overbar = 10.0;
    int zeta_SMT = 200;
    double epsilon = 0.0;
    int grid_points = 50;
    int zeta_D = 50;

    double alpha_1 = 1.0;
    double alpha_2 = 1.0;
    double alpha_3 = 1.0;
    double alpha_4 = 0.0;
    double alpha_roa = 1.0;
    double alpha_5 = 1.0;

    int n1 = 1;
    int n2 = 30;
    double K = 0.478;
    int T = 216;
    int d = 0;

    bool execute_postprocessing = true;
    bool verbose_info = true;
    int dpi_ = 300;
    bool plot_V = true;
    bool plot_Vdot = true;
    bool plot_u = true;
    bool plot_4D_ = true;
    int n_points_4D = 500;
    int n_points_3D = 100;
    bool plot_ctr_weights = true;
    bool plot_V_weights = true;
    bool plot_dataset = true;

    bool test_closed_loop_dynamics = true;
    double end_time = 300.0;
    double Dt = 0.01;

    // 转换为JSON对象
    QJsonObject toJson() const;
    // 从JSON对象加载
    void fromJson(const QJsonObject& json);
};

// 测试记录结构体
struct TestRecord {
    int test_id = -1;
    int UUID = -1;
    int config_id = -1;
    QString test_name;
    QString test_code;
    QString params_detail;  // JSON字符串
    QString result_path;
    QString metrics_data;   // JSON字符串
    QDateTime execute_time;
    QString remark;
};

class TestDbHelper : public QObject
{
    Q_OBJECT
public:
    static TestDbHelper* getInstance();
    ~TestDbHelper();

    // 配置参数相关
    bool saveConfigParams(const ConfigParams& params, int UUID, int& configId); // 保存配置，返回是否成功，输出configId
    bool getConfigParams(int UUID, int configId, ConfigParams& params);         // 根据ID获取配置

    // 测试记录相关
    bool addTestRecord(const TestRecord& record);                    // 添加测试记录
    bool updateTestRecord(const TestRecord& record);                 // 更新测试记录
    bool deleteTestRecord(int testId);                               // 删除测试记录
    QList<TestRecord> getAllTestRecords(int UUID);                           // 获取所有测试记录

private:
    TestDbHelper(QObject *parent = nullptr);
    static TestDbHelper* m_instance;
    static QMutex m_mutex;
    BaseDbHelper* m_dbHelper;
};

#endif // TESTDBHELPER_H
