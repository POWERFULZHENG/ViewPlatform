#include "TestDbHelper.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

// ConfigParams 转换JSON
QJsonObject ConfigParams::toJson() const {
    QJsonObject json;
    // campaign_params
    json["init_seed"] = init_seed;
    json["campaign_run"] = campaign_run;
    json["tot_runs"] = tot_runs;
    json["max_loop_number"] = max_loop_number;
    json["max_iters"] = max_iters;
    json["system_name"] = system_name;
    json["x_star"] = QJsonArray({x_star_1, x_star_2});

    // learner_params
    json["N"] = N;
    json["N_max"] = N_max;
    json["sliding_window"] = sliding_window;
    json["learning_rate"] = learning_rate;
    json["learning_rate_c"] = learning_rate_c;
    json["use_scheduler"] = use_scheduler;
    json["sched_T"] = sched_T;
    json["print_interval"] = print_interval;

    // lyap_params
    json["n_input"] = n_input;
    json["beta_sfpl"] = beta_sfpl;
    json["clipping_V"] = clipping_V;
    json["size_layers"] = size_layers;
    json["lyap_activations"] = lyap_activations;
    json["lyap_bias"] = lyap_bias;

    // control_params
    json["use_lin_ctr"] = use_lin_ctr;
    json["lin_contr_bias"] = lin_contr_bias;
    json["control_initialised"] = control_initialised;
    json["init_control"] = init_control;
    json["size_ctrl_layers"] = size_ctrl_layers;
    json["ctrl_bias"] = ctrl_bias;
    json["ctrl_activations"] = ctrl_activations;
    json["use_saturation"] = use_saturation;
    json["ctrl_sat"] = ctrl_sat;

    // falsifier_params
    json["gamma_underbar"] = gamma_underbar;
    json["gamma_overbar"] = gamma_overbar;
    json["zeta_SMT"] = zeta_SMT;
    json["epsilon"] = epsilon;
    json["grid_points"] = grid_points;
    json["zeta_D"] = zeta_D;

    // loss_function
    json["alpha_1"] = alpha_1;
    json["alpha_2"] = alpha_2;
    json["alpha_3"] = alpha_3;
    json["alpha_4"] = alpha_4;
    json["alpha_roa"] = alpha_roa;
    json["alpha_5"] = alpha_5;

    // dyn_sys_params
    json["n1"] = n1;
    json["n2"] = n2;
    json["K"] = K;
    json["T"] = T;
    json["d"] = d;

    // postproc_params
    json["execute_postprocessing"] = execute_postprocessing;
    json["verbose_info"] = verbose_info;
    json["dpi_"] = dpi_;
    json["plot_V"] = plot_V;
    json["plot_Vdot"] = plot_Vdot;
    json["plot_u"] = plot_u;
    json["plot_4D_"] = plot_4D_;
    json["n_points_4D"] = n_points_4D;
    json["n_points_3D"] = n_points_3D;
    json["plot_ctr_weights"] = plot_ctr_weights;
    json["plot_V_weights"] = plot_V_weights;
    json["plot_dataset"] = plot_dataset;

    // closed_loop_params
    json["test_closed_loop_dynamics"] = test_closed_loop_dynamics;
    json["end_time"] = end_time;
    json["Dt"] = Dt;

    return json;
}

void ConfigParams::fromJson(const QJsonObject& json) {
    // 仅示例核心参数，其余参数可按需补充
    init_seed = json["init_seed"].toInt(1);
    campaign_run = json["campaign_run"].toInt(2000);
    system_name = json["system_name"].toString("nomoto_ship");

    QJsonArray xStar = json["x_star"].toArray();
    if (xStar.size() >= 2) {
        x_star_1 = xStar[0].toDouble(30.0);
        x_star_2 = xStar[1].toDouble(0.0);
    }

    learning_rate = json["learning_rate"].toDouble(0.01);
    use_saturation = json["use_saturation"].toBool(true);
    K = json["K"].toDouble(0.478);
    end_time = json["end_time"].toDouble(300.0);
}

// TestDbHelper 单例实现
TestDbHelper* TestDbHelper::m_instance = nullptr;
QMutex TestDbHelper::m_mutex;

TestDbHelper::TestDbHelper(QObject *parent) : QObject(parent) {
    m_dbHelper = BaseDbHelper::getInstance();
}

TestDbHelper::~TestDbHelper() {
    // 释放资源
}

TestDbHelper* TestDbHelper::getInstance() {
    if (m_instance == nullptr) {
        m_mutex.lock();
        if (m_instance == nullptr) {
            m_instance = new TestDbHelper();
        }
        m_mutex.unlock();
    }
    return m_instance;
}

// 保存配置参数
bool TestDbHelper::saveConfigParams(const ConfigParams& params, int UUID, int& configId) {
    QString sql = R"(
        INSERT INTO config_params (
            user_id, init_seed, campaign_run, tot_runs, max_loop_number, max_iters, system_name,
            x_star_1, x_star_2, N, N_max, sliding_window, learning_rate, learning_rate_c,
            use_scheduler, sched_T, print_interval, n_input, beta_sfpl, clipping_V,
            size_layers, lyap_activations, lyap_bias, use_lin_ctr, lin_contr_bias,
            control_initialised, init_control, size_ctrl_layers, ctrl_bias, ctrl_activations,
            use_saturation, ctrl_sat, gamma_underbar, gamma_overbar, zeta_SMT, epsilon,
            grid_points, zeta_D, alpha_1, alpha_2, alpha_3, alpha_4, alpha_roa, alpha_5,
            n1, n2, K, T, d, execute_postprocessing, verbose_info, dpi_, plot_V, plot_Vdot,
            plot_u, plot_4D_, n_points_4D, n_points_3D, plot_ctr_weights, plot_V_weights,
            plot_dataset, test_closed_loop_dynamics, end_time, Dt
        ) VALUES (
            ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
            ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
            ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
        )
    )";

    QVariantList paramsList = {
        UUID, params.init_seed, params.campaign_run, params.tot_runs, params.max_loop_number,
        params.max_iters, params.system_name, params.x_star_1, params.x_star_2, params.N,
        params.N_max, params.sliding_window, params.learning_rate, params.learning_rate_c,
        params.use_scheduler, params.sched_T, params.print_interval, params.n_input,
        params.beta_sfpl, params.clipping_V, params.size_layers, params.lyap_activations,
        params.lyap_bias, params.use_lin_ctr, params.lin_contr_bias, params.control_initialised,
        params.init_control, params.size_ctrl_layers, params.ctrl_bias, params.ctrl_activations,
        params.use_saturation, params.ctrl_sat, params.gamma_underbar, params.gamma_overbar,
        params.zeta_SMT, params.epsilon, params.grid_points, params.zeta_D, params.alpha_1,
        params.alpha_2, params.alpha_3, params.alpha_4, params.alpha_roa, params.alpha_5,
        params.n1, params.n2, params.K, params.T, params.d, params.execute_postprocessing,
        params.verbose_info, params.dpi_, params.plot_V, params.plot_Vdot, params.plot_u,
        params.plot_4D_, params.n_points_4D, params.n_points_3D, params.plot_ctr_weights,
        params.plot_V_weights, params.plot_dataset, params.test_closed_loop_dynamics,
        params.end_time, params.Dt
    };

    if (!m_dbHelper->execPrepareSql(sql, paramsList)) {
        qCritical() << "保存配置参数失败：" << m_dbHelper->getLastError();
        return false;
    }

    // 获取插入的ID
    QSqlQuery query = m_dbHelper->execQuery("SELECT LAST_INSERT_ID()");
    if (query.next()) {
        configId = query.value(0).toInt();
    } else {
        configId = -1;
        return false;
    }

    return true;
}

// 获取配置参数
bool TestDbHelper::getConfigParams(int UUID, int configId, ConfigParams& params) {
    QString sql = "SELECT * FROM config_params WHERE config_id = ?";
    QSqlQuery query = m_dbHelper->execPrepareQuery(sql, {UUID, configId});
    if (!query.next()) {
        qCritical() << "获取配置参数失败，配置ID或者用户ID不存在：" << UUID << ":" << configId;
        return false;
    }

    // 映射数据库字段到结构体
    params.user_id = query.value("user_id").toInt();
    params.init_seed = query.value("init_seed").toInt();
    params.campaign_run = query.value("campaign_run").toInt();
    params.tot_runs = query.value("tot_runs").toInt();
    params.max_loop_number = query.value("max_loop_number").toInt();
    params.max_iters = query.value("max_iters").toInt();
    params.system_name = query.value("system_name").toString();
    params.x_star_1 = query.value("x_star_1").toDouble();
    params.x_star_2 = query.value("x_star_2").toDouble();

    params.N = query.value("N").toInt();
    params.N_max = query.value("N_max").toInt();
    params.sliding_window = query.value("sliding_window").toBool();
    params.learning_rate = query.value("learning_rate").toDouble();
    params.learning_rate_c = query.value("learning_rate_c").toDouble();
    params.use_scheduler = query.value("use_scheduler").toBool();
    params.sched_T = query.value("sched_T").toInt();
    params.print_interval = query.value("print_interval").toInt();

    params.n_input = query.value("n_input").toInt();
    params.beta_sfpl = query.value("beta_sfpl").toDouble();
    params.clipping_V = query.value("clipping_V").toBool();
    params.size_layers = query.value("size_layers").toString();
    params.lyap_activations = query.value("lyap_activations").toString();
    params.lyap_bias = query.value("lyap_bias").toString();

    params.use_lin_ctr = query.value("use_lin_ctr").toBool();
    params.lin_contr_bias = query.value("lin_contr_bias").toBool();
    params.control_initialised = query.value("control_initialised").toBool();
    params.init_control = query.value("init_control").toString();
    params.size_ctrl_layers = query.value("size_ctrl_layers").toString();
    params.ctrl_bias = query.value("ctrl_bias").toString();
    params.ctrl_activations = query.value("ctrl_activations").toString();
    params.use_saturation = query.value("use_saturation").toBool();
    params.ctrl_sat = query.value("ctrl_sat").toString();

    params.gamma_underbar = query.value("gamma_underbar").toDouble();
    params.gamma_overbar = query.value("gamma_overbar").toDouble();
    params.zeta_SMT = query.value("zeta_SMT").toInt();
    params.epsilon = query.value("epsilon").toDouble();
    params.grid_points = query.value("grid_points").toInt();
    params.zeta_D = query.value("zeta_D").toInt();

    params.alpha_1 = query.value("alpha_1").toDouble();
    params.alpha_2 = query.value("alpha_2").toDouble();
    params.alpha_3 = query.value("alpha_3").toDouble();
    params.alpha_4 = query.value("alpha_4").toDouble();
    params.alpha_roa = query.value("alpha_roa").toDouble();
    params.alpha_5 = query.value("alpha_5").toDouble();

    params.n1 = query.value("n1").toInt();
    params.n2 = query.value("n2").toInt();
    params.K = query.value("K").toDouble();
    params.T = query.value("T").toInt();
    params.d = query.value("d").toInt();

    params.execute_postprocessing = query.value("execute_postprocessing").toBool();
    params.verbose_info = query.value("verbose_info").toBool();
    params.dpi_ = query.value("dpi_").toInt();
    params.plot_V = query.value("plot_V").toBool();
    params.plot_Vdot = query.value("plot_Vdot").toBool();
    params.plot_u = query.value("plot_u").toBool();
    params.plot_4D_ = query.value("plot_4D_").toBool();
    params.n_points_4D = query.value("n_points_4D").toInt();
    params.n_points_3D = query.value("n_points_3D").toInt();
    params.plot_ctr_weights = query.value("plot_ctr_weights").toBool();
    params.plot_V_weights = query.value("plot_V_weights").toBool();
    params.plot_dataset = query.value("plot_dataset").toBool();

    params.test_closed_loop_dynamics = query.value("test_closed_loop_dynamics").toBool();
    params.end_time = query.value("end_time").toDouble();
    params.Dt = query.value("Dt").toDouble();

    return true;
}

// 添加测试记录
bool TestDbHelper::addTestRecord(const TestRecord& record) {
    QString sql = R"(
        INSERT INTO test_records (
            user_id, config_id, test_name, test_code, params_detail, result_path, metrics_data,
            execute_time, remark
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

    QVariantList paramsList = {
        record.UUID, record.config_id, record.test_name, record.test_code, record.params_detail,
        record.result_path, record.metrics_data, record.execute_time.toString("yyyy-MM-dd HH:mm:ss"),
        record.remark
    };

    if (!m_dbHelper->execPrepareSql(sql, paramsList)) {
        qCritical() << "添加测试记录失败：" << m_dbHelper->getLastError();
        return false;
    }
    return true;
}

// 更新测试记录
bool TestDbHelper::updateTestRecord(const TestRecord& record) {
    QString sql = R"(
        UPDATE test_records SET
            user_id = ?, config_id = ?, test_name = ?, test_code = ?, params_detail = ?, result_path = ?,
            metrics_data = ?, execute_time = ?, remark = ?
        WHERE test_id = ?
    )";
    QVariantList paramsList = {
        record.UUID, record.config_id, record.test_name, record.test_code, record.params_detail,
        record.result_path, record.metrics_data, record.execute_time.toString("yyyy-MM-dd HH:mm:ss"),
        record.remark, record.test_id
    };

    if (!m_dbHelper->execPrepareSql(sql, paramsList)) {
        qCritical() << "更新测试记录失败：" << m_dbHelper->getLastError();
        return false;
    }
    return true;
}

// 删除测试记录
bool TestDbHelper::deleteTestRecord(int testId) {
    QString sql = "DELETE FROM test_records WHERE test_id = ?";
    if (!m_dbHelper->execPrepareSql(sql, {testId})) {
        qCritical() << "删除测试记录失败：" << m_dbHelper->getLastError();
        return false;
    }
    return true;
}

// 获取所有测试记录
QList<TestRecord> TestDbHelper::getAllTestRecords(int UUID) {
    QList<TestRecord> records;
    QString sql = "SELECT * FROM test_records WHERE user_id = ? ORDER BY test_id DESC";
    QSqlQuery query = m_dbHelper->execPrepareQuery(sql, {UUID});

    while (query.next()) {
        TestRecord record;
        record.test_id = query.value("test_id").toInt();
        record.UUID = query.value("user_id").toInt();
        record.config_id = query.value("config_id").toInt();
        record.test_name = query.value("test_name").toString();
        record.test_code = query.value("test_code").toString();
        record.params_detail = query.value("params_detail").toString();
        record.result_path = query.value("result_path").toString();
        record.metrics_data = query.value("metrics_data").toString();
        record.execute_time = QDateTime::fromString(query.value("execute_time").toString(), "yyyy-MM-dd'T'HH:mm:ss.zzz");
        record.remark = query.value("remark").toString();
//        qDebug() << record.execute_time << query.value("execute_time").toString();
        records.append(record);
    }

    return records;
}

QList<TestRecord> TestDbHelper::getAllTestRecords() {
    QList<TestRecord> records;
    QString sql = "SELECT * FROM test_records ORDER BY test_id DESC";
    QSqlQuery query = m_dbHelper->execPrepareQuery(sql, {});

    while (query.next()) {
        TestRecord record;
        record.test_id = query.value("test_id").toInt();
        record.UUID = query.value("user_id").toInt();
        record.config_id = query.value("config_id").toInt();
        record.test_name = query.value("test_name").toString();
        record.test_code = query.value("test_code").toString();
        record.params_detail = query.value("params_detail").toString();
        record.result_path = query.value("result_path").toString();
        record.metrics_data = query.value("metrics_data").toString();
        record.execute_time = QDateTime::fromString(query.value("execute_time").toString(), "yyyy-MM-dd'T'HH:mm:ss.zzz");
        record.remark = query.value("remark").toString();
//        qDebug() << record.execute_time << query.value("execute_time").toString();
        records.append(record);
    }

    return records;
}
