CREATE TABLE IF NOT EXISTS sys_log (
    log_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '日志唯一标识',
    user_id INT NOT NULL COMMENT '关联用户ID',
    operation_type VARCHAR(20) NOT NULL COMMENT '操作类型（如：login, create, update, delete）',
    operation_content TEXT COMMENT '操作内容（详细描述）',
    ip_address VARCHAR(15) NOT NULL COMMENT 'IP地址（IPv4格式或者IPv6）',
    log_level VARCHAR(20) DEFAULT 'INFO' COMMENT '日志级别(INFO/WARNING/ERROR/DEBUG)',
    module_name VARCHAR(50) NOT NULL COMMENT '模块名称',
    create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '记录创建时间',
    FOREIGN KEY (user_id) REFERENCES sys_user(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='系统操作日志表';
