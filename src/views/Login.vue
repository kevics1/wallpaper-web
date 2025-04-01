<template>
  <div class="login-container">
    <div class="login-header">
      <h1 class="login-title">欢迎登录系统</h1>
    </div>

    <el-form :model="loginForm" :rules="loginRules" ref="loginForm" label-position="left" class="login-form">
      <el-form-item label="用户名" prop="username">
        <el-input prefix-icon="el-icon-user-solid" v-model="loginForm.username" auto-complete="off" placeholder="请输入用户名"
          style="width: 100%;"></el-input>
      </el-form-item>

      <el-form-item label="密码" prop="password">
        <el-input prefix-icon="el-icon-lock" type="password" v-model="loginForm.passwordHash" auto-complete="off"
          placeholder="请输入密码" style="width: 100%;"></el-input>
      </el-form-item>

      <el-form-item>
        <el-button type="primary" native-type="submit" @click="handleLogin">登录</el-button>
        <span class="forgot-password">忘记密码？</span>
      </el-form-item>
    </el-form>

    <div class="login-footer">
      <p>还没有账号？<router-link to="/signup">立即注册</router-link></p>
    </div>
  </div>
</template>


<script>
import { doLogin } from '@/api/user';

export default {
  data() {
    return {
      loginForm: {
        username: '',
        passwordHash: ''
      },
      loginRules: {
        username: [
          { required: true, message: '请输入用户名', trigger: 'blur' }
        ],
        passwordHash: [
          { required: true, message: '请输入密码', trigger: 'blur' }
        ]
      }
    };
  },
  methods: {
    async login() {
      const { username, passwordHash } = this.loginForm;
      await doLogin({ username, passwordHash }).then(response => {
        if (response.code == 200) {
          // 登录成功，跳转至主页或其他操作
          this.$router.push({ name: 'home' })
          // alert(response.msg);
        } else {
          this.$errorMessage(response.msg);
        }
      });
    },
    handleLogin() {
      this.login();
    }
  }
};
</script>
<style scoped>
.login-container {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100vh;
  background-color: #f0f2f5;
}

.login-header {
  text-align: center;
  padding: 30px 0;
}

.login-title {
  font-size: 24px;
  color: #333;
}

.login-form {
  width: 360px;
  border-radius: 4px;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.12), 0 1px 2px rgba(0, 0, 0, 0.24);
  background-color: #fff;
  padding: 30px;
}

.login-form .el-input__prefix {
  color: #ccc;
}

.forgot-password {
  float: right;
  color: #999;
  cursor: pointer;
}

.login-footer {
  text-align: center;
  margin-top: 30px;
  color: #999;
}

.login-footer a {
  color: #409eff;
  text-decoration: none;
}
</style>