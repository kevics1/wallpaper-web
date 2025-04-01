const { defineConfig } = require('@vue/cli-service')
module.exports = defineConfig({
  transpileDependencies: true,
  lintOnSave: false,    //添加此项v
  devServer: {
    historyApiFallback: true,
    allowedHosts: "all",
        }
})
