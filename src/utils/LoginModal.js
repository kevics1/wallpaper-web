import Vue from 'vue';
import LoginModal from '../components/LoginModal.vue';

const PopupBox = Vue.extend(LoginModal);

// LoginModal.install = function (data) {
//   let instance = new PopupBox({
//     data
//   }).$mount(document.createElement('div'));

//   document.body.appendChild(instance.$el);

//   Object.assign(instance, data);

//   Vue.nextTick(() => {
//     instance.show = true
//   });
// }

// export default LoginModal;

export function showLoginModal() {
  let instance = new PopupBox().$mount(document.createElement('div'));
  document.body.appendChild(instance.$el);
  Vue.nextTick(() => {
      instance.show = true;
  });
  return instance;
}