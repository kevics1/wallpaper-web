export default {
  install(Vue) {
    // 错误弹窗信息
    Vue.prototype.$errorMessage = function(text) {
      this.$message({
        message: text,
        type: 'error'
      });
    };

    // 成功弹窗信息
    Vue.prototype.$successMessage = function(text) {
      this.$message({
        message: text,
        type: 'success'
      });
    };
  }
};