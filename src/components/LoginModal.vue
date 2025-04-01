<template>
  <transition name="fade">
    <div class="mask" v-if="show">
      <div class="modal">
        <div class="modal-header">
          <h3>登录/注册</h3>
        </div>

        <!-- 登录/注册选项 -->
        <div class="modal-options">
          <el-radio-group v-model="selectedOption" @change="handleOptionChange">
            <el-radio-button label="login">登录</el-radio-button>
            <el-radio-button label="registerAndLogin">注册并登录</el-radio-button>
          </el-radio-group>
        </div>

        <!-- 显示错误信息 -->
        <div class="error-message-container" v-if="errorMessage">
          <i class="el-icon-warning"></i>
          <span>{{ errorMessage }}</span>
        </div>


        <!-- 根据选项显示对应的表单 -->
        <div class="modal-body" v-show="selectedOption === 'login'">
          <el-input type="text" v-model="username" placeholder="请输入用户名" />
          <el-input type="password" v-model="password" placeholder="请输入密码" />
        </div>

        <!-- 注册表单，可以进一步扩展以收集更多注册信息 -->
        <div class="modal-body register-form" v-show="selectedOption === 'registerAndLogin'">
          <el-input type="text" v-model="registerUsername" placeholder="请输入用户名" />
          <el-input type="password" v-model="registerPassword" placeholder="请设置密码" />
          <el-input type="password" v-model="confirmPassword" placeholder="再次确认密码" />
        </div>

        <div class="modal-footer">
          <button @click="handleOk(selectedOption)">确认</button>
          <button @click="show = false">取消</button>
        </div>
      </div>
    </div>
  </transition>
</template>

<script>
import { EventBus } from '@/utils/event-bus.js';
import { doLogin } from '@/api/user';
import { doRegister } from '@/api/user';

export default {
  name: 'LoginModal',
  data() {
    return {
      show: true,
      selectedOption: 'login', // 默认显示登录表单
      username: '',
      password: '',
      registerUsername: '',
      registerPassword: '',
      confirmPassword: '',
      errorMessage: '',
      currentUser: {},
    };
  },
  methods: {
    async login() {
      const username = this.username;
      const passwordHash = this.password;

      await doLogin({ username, passwordHash }).then((response) => {
        if (response.code === 200) {
          this.currentUser = response.data;
          this.saveUserData();
          this.resetForm();
        } else {
          this.errorMessage = response.msg;
        }
      });
    },
    async registerAndLogin() {
      const username = this.registerUsername;
      const passwordHash = this.registerPassword;
      if (passwordHash != this.confirmPassword) {
        console.log('password:' + passwordHash + 'confirm:' + this.confirmPassword)
        this.errorMessage = '两次输入的密码不一致';
        return;
      }

      await doRegister({ username, passwordHash }).then(response => {
        if (response.code == 200) {
          this.currentUser = response.data;
          this.saveUserData();
          this.resetForm();
        } else {
          this.errorMessage = response.msg;
        }
      });

    },
    saveUserData() {
      // 存储用户信息到localStorage
      localStorage.setItem('userData', JSON.stringify(this.currentUser));
      localStorage.setItem('satoken', this.currentUser.saToken);
      this.$emit('login-success', this.currentUser); // 通知Navbar登录成功
      EventBus.$emit('login-success', this.currentUser);
    },
    handleOk(option) {
      if (option === 'login') {
        this.login();
      } else if (option === 'registerAndLogin') {
        this.registerAndLogin();
      }
    },
    handleOptionChange(newOption) {
      // 切换表单时清空之前表单的输入内容
      if (newOption === 'login') {
        this.registerUsername = '';
        this.registerPassword = '';
      } else if (newOption === 'registerAndLogin') {
        this.username = '';
        this.password = '';
      }
    },
    resetForm() {
      // 清空表单数据
      this.username = '';
      this.password = '';
      this.registerUsername = '';
      this.registerPassword = '';
      this.show = false;
    }
  }
};
</script>

<style scoped>
.mask {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  display: flex;
  justify-content: center;
  align-items: center;
  background-color: rgba(0, 0, 0, 0.6);
  /* 蒙版背景色 */
  z-index: 1000;
  /* 确保蒙版在最上层 */
}

.modal {
  background: #fff;
  padding: 20px;
  border-radius: 8px;
  box-shadow: 0 4px 10px rgba(0, 0, 0, 0.2);
  width: 400px;
  /* 弹窗宽度 */
  max-width: 90%;
  /* 适应小屏幕 */
  text-align: center;
  /* 文本居中 */
}

.modal-header {
  margin-bottom: 20px;
}

.modal-header h3 {
  margin: 0;
  color: #333;
  font-size: 1.5em;
}

.modal-body {
  margin-bottom: 20px;
}

input[type="text"],
input[type="password"] {
  width: 100%;
  padding: 10px;
  margin-bottom: 10px;
  border: 1px solid #ccc;
  border-radius: 4px;
  box-sizing: border-box;
  /* 防止输入框超出容器宽度 */
}

.modal-footer {
  text-align: right;
}

button {
  padding: 10px 20px;
  margin-left: 10px;
  border: none;
  border-radius: 4px;
  background-color: #0BE1FD;
  /* 按钮背景色 */
  color: white;
  cursor: pointer;
  transition: background-color 0.3s ease;
}

button:hover {
  background-color: #03c5df;
  /* 鼠标悬停时的背景色 */
}

.close {
  position: absolute;
  top: 10px;
  right: 10px;
  cursor: pointer;
  transition: transform 0.3s ease;
}

.close:hover {
  transform: rotate(90deg);
  /* 鼠标悬停时旋转关闭图标 */
}

.modal-options {
  margin-bottom: 20px;
}

/* 添加额外的输入框间距样式 */
.modal-body .el-input {
  margin-bottom: 10px;
}

.error-message-container {
  display: flex;
  align-items: center;
  color: #f56c6c;
  /* 或其他警示颜色 */
  font-size: 0.9em;
  margin-top: 10px;
  margin-bottom: 10px;
  padding: 6px 12px;
  border-radius: 4px;
  background-color: rgba(245, 108, 108, 0.1);
  /* 背景半透明颜色 */
}

.error-message-container i {
  margin-right: 6px;
  font-size: 16px;
}
</style>