CREATE DATABASE IF NOT EXISTS viewplatfrom DEFAULT CHARACTER SET utf8mb4;
USE viewplatfrom;

-- 日志表：主键、非空约束和合理数据类型
CREATE TABLE IF NOT EXISTS sys_log (
    log_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '日志唯一标识',
    user_id INT NOT NULL,
    operator VARCHAR(50) NOT NULL COMMENT '操作人（用户名或ID）',
    operation_type VARCHAR(20) NOT NULL COMMENT '操作类型（如：login, create, update, delete）',
    operation_content TEXT COMMENT '操作内容（详细描述，可为空）',
    operation_time DATETIME NOT NULL COMMENT '操作时间（精确到秒）',
    ip_address VARCHAR(15) NOT NULL COMMENT 'IP地址（IPv4格式）',
    FOREIGN KEY (user_id) REFERENCES sys_user(id) ON DELETE CASCADE
) COMMENT='系统操作日志表';


-- 用户表：存储手机号(唯一)、密
CREATE TABLE IF NOT EXISTS sys_user (
  id INT PRIMARY KEY AUTO_INCREMENT COMMENT '用户ID',
  user_name VARCHAR(50) NOT NULL COMMENT '登录账号',
  nick_name VARCHAR(50) NOT NULL COMMENT '用户昵称',
  role_name VARCHAR(20) NOT NULL COMMENT '用户角色',
  phone VARCHAR(20) NOT NULL COMMENT '手机号',
  pwd VARCHAR(255) NOT NULL COMMENT '用户登录密码',
  status TINYINT DEFAULT 1 COMMENT '0=禁用 1=启用',
  create_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间'
)COMMENT='系统用户表';

-- 配置参数表（保存Python脚本的所有参数）
CREATE TABLE IF NOT EXISTS config_params (
    config_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '配置ID',
    user_id INT NOT NULL,
    -- campaign_params
    init_seed INT NOT NULL DEFAULT 1,
    campaign_run INT NOT NULL DEFAULT 2000,
    tot_runs INT NOT NULL DEFAULT 20,
    max_loop_number INT NOT NULL DEFAULT 1,
    max_iters INT NOT NULL DEFAULT 5000,
    system_name VARCHAR(50) NOT NULL DEFAULT 'nomoto_ship',
    x_star_1 DOUBLE NOT NULL DEFAULT 30.0,
    x_star_2 DOUBLE NOT NULL DEFAULT 0.0,
    -- learner_params
    N INT NOT NULL DEFAULT 500,
    N_max INT NOT NULL DEFAULT 1000,
    sliding_window TINYINT NOT NULL DEFAULT 1,
    learning_rate DOUBLE NOT NULL DEFAULT 0.01,
    learning_rate_c DOUBLE NOT NULL DEFAULT 0.1,
    use_scheduler TINYINT NOT NULL DEFAULT 1,
    sched_T INT NOT NULL DEFAULT 300,
    print_interval INT NOT NULL DEFAULT 200,
    -- lyap_params
    n_input INT NOT NULL DEFAULT 2,
    beta_sfpl DOUBLE NOT NULL DEFAULT 2,
    clipping_V TINYINT NOT NULL DEFAULT 1,
    size_layers VARCHAR(50) NOT NULL DEFAULT '[10,10,1]',
    lyap_activations VARCHAR(50) NOT NULL DEFAULT '["pow2","linear","linear"]',
    lyap_bias VARCHAR(50) NOT NULL DEFAULT '[false,false,false]',
    -- control_params
    use_lin_ctr TINYINT NOT NULL DEFAULT 0,
    lin_contr_bias TINYINT NOT NULL DEFAULT 0,
    control_initialised TINYINT NOT NULL DEFAULT 0,
    init_control VARCHAR(50) NOT NULL DEFAULT '[[-23.58639732, -5.31421063]]',
    size_ctrl_layers VARCHAR(50) NOT NULL DEFAULT '[50,1]',
    ctrl_bias VARCHAR(50) NOT NULL DEFAULT '[true,false]',
    ctrl_activations VARCHAR(50) NOT NULL DEFAULT '["tanh","linear"]',
    use_saturation TINYINT NOT NULL DEFAULT 1,
    ctrl_sat VARCHAR(50) NOT NULL DEFAULT '[35.0]',
    -- falsifier_params
    gamma_underbar DOUBLE NOT NULL DEFAULT 0.1,
    gamma_overbar DOUBLE NOT NULL DEFAULT 10.0,
    zeta_SMT INT NOT NULL DEFAULT 200,
    epsilon DOUBLE NOT NULL DEFAULT 0.0,
    grid_points INT NOT NULL DEFAULT 50,
    zeta_D INT NOT NULL DEFAULT 50,
    -- loss_function
    alpha_1 DOUBLE NOT NULL DEFAULT 1.0,
    alpha_2 DOUBLE NOT NULL DEFAULT 1.0,
    alpha_3 DOUBLE NOT NULL DEFAULT 1.0,
    alpha_4 DOUBLE NOT NULL DEFAULT 0.0,
    alpha_roa DOUBLE NOT NULL DEFAULT 1.0,
    alpha_5 DOUBLE NOT NULL DEFAULT 1.0,
    -- dyn_sys_params
    n1 INT NOT NULL DEFAULT 1,
    n2 INT NOT NULL DEFAULT 30,
    K DOUBLE NOT NULL DEFAULT 0.478,
    T INT NOT NULL DEFAULT 216,
    d INT NOT NULL DEFAULT 0,
    -- postproc_params
    execute_postprocessing TINYINT NOT NULL DEFAULT 1,
    verbose_info TINYINT NOT NULL DEFAULT 1,
    dpi_ INT NOT NULL DEFAULT 300,
    plot_V TINYINT NOT NULL DEFAULT 1,
    plot_Vdot TINYINT NOT NULL DEFAULT 1,
    plot_u TINYINT NOT NULL DEFAULT 1,
    plot_4D_ TINYINT NOT NULL DEFAULT 1,
    n_points_4D INT NOT NULL DEFAULT 500,
    n_points_3D INT NOT NULL DEFAULT 100,
    plot_ctr_weights TINYINT NOT NULL DEFAULT 1,
    plot_V_weights TINYINT NOT NULL DEFAULT 1,
    plot_dataset TINYINT NOT NULL DEFAULT 1,
    -- closed_loop_params
    test_closed_loop_dynamics TINYINT NOT NULL DEFAULT 1,
    end_time DOUBLE NOT NULL DEFAULT 300.0,
    Dt DOUBLE NOT NULL DEFAULT 0.01,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES sys_user(id) ON DELETE CASCADE
);

-- 测试记录表（对应界面表格）
CREATE TABLE IF NOT EXISTS test_records (
    test_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '序号',
    test_name VARCHAR(100) NOT NULL COMMENT '测试名称',
    test_code VARCHAR(50) NOT NULL COMMENT '测试代号',
    params_detail TEXT COMMENT '参数详情（JSON格式）',
    result_path VARCHAR(255) COMMENT '结果文件夹路径',
    metrics_data TEXT COMMENT '指标数据（JSON格式）',
    execute_time DATETIME COMMENT '执行完成时间',
    remark VARCHAR(255) DEFAULT '' COMMENT '备注',
    config_id INT COMMENT '关联的配置ID',
    FOREIGN KEY (config_id) REFERENCES config_params(config_id)
    FOREIGN KEY (user_id) REFERENCES sys_user(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS chat_dialog (
  id INT PRIMARY KEY AUTO_INCREMENT COMMENT '对话ID',
  user_id INT NOT NULL COMMENT '关联user表id',
  dialog_title VARCHAR(100) NOT NULL COMMENT '对话标题',
  dialog_content TEXT NOT NULL COMMENT 'JSON格式存储多轮对话',
  create_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  update_time DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  FOREIGN KEY (user_id) REFERENCES sys_user(id) ON DELETE CASCADE
) COMMENT='大模型对话记录表';

-- 用户API配置表（多用户独立配置）
CREATE TABLE IF NOT EXISTS user_api_config (
  id INT PRIMARY KEY AUTO_INCREMENT COMMENT '配置ID',
  user_id INT NOT NULL UNIQUE COMMENT '关联user表id（唯一）',
  api_url VARCHAR(255) NOT NULL DEFAULT 'https://open.bigmodel.cn/api/paas/v4/chat/completions' COMMENT '智谱AI接口地址',
  api_key VARCHAR(255) NOT NULL COMMENT 'API密钥',
  model VARCHAR(50) NOT NULL DEFAULT 'glm-4.7' COMMENT '模型版本',
  temperature FLOAT NOT NULL DEFAULT 1.0 COMMENT '温度参数',
  create_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  update_time DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  FOREIGN KEY (user_id) REFERENCES sys_user(id) ON DELETE CASCADE
) COMMENT='用户大模型API配置表';

-- 系统大模型配置表（可自定义添加模型）
CREATE TABLE IF NOT EXISTS model_config (
  id INT PRIMARY KEY AUTO_INCREMENT COMMENT '模型ID',
  user_id INT NOT NULL,
  model_code VARCHAR(50) NOT NULL UNIQUE COMMENT '模型编码（如glm-4.7）',
  model_name VARCHAR(100) NOT NULL COMMENT '模型名称（如智谱GLM-4.7）',
  is_default TINYINT DEFAULT 0 COMMENT '是否默认模型（1=是）',
  create_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  FOREIGN KEY (user_id) REFERENCES sys_user(id) ON DELETE CASCADE
) COMMENT='系统大模型配置表';

-- 初始化默认模型（智谱系列）
INSERT INTO model_config (model_code, model_name, is_default) 
VALUES 
('glm-4.7', '智谱GLM-4.7', 1),
('glm-4', '智谱GLM-4', 0),
('glm-3-turbo', '智谱GLM-3-Turbo', 0)
ON DUPLICATE KEY UPDATE model_name = VALUES(model_name);