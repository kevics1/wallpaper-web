import Vue from 'vue'
import ElementUI from 'element-ui';
import 'element-ui/lib/theme-chalk/index.css';
import App from './App.vue'
import './plugins/element.js'
import router from './router/index.js'; 
import './element-ui.scss'
import msgTools from './utils/messageTools.js';
// import LoginModal from './utils/LoginModal.js';

// Vue.prototype.$loginModal = LoginModal.install
Vue.config.productionTip = false

Vue.use(ElementUI);
Vue.use(msgTools);


new Vue({
  router,
  render: h => h(App),
}).$mount('#app')
