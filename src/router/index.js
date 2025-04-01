import Vue from 'vue';
import Router from 'vue-router';

// 引入页面组件
import Home from '@/views/Home.vue';
import Categories from '@/views/Categories.vue';
import Wallpaper from '@/views/Wallpaper.vue';
import Search from '@/views/Search.vue';
import Upload from '@/views/Upload.vue';
import Profile from '@/views/Profile.vue';
import Login from '@/views/Login.vue';
import Register from '@/views/Register.vue';
import Forum from '@/views/Forum.vue';
import Leave from '@/views/Leave.vue';
import Anenst from '@/views/Anenst.vue';

Vue.use(Router);

export default new Router({
  mode: 'history', // 使用history模式，避免URL中的#
  base: process.env.BASE_URL, // 基础路径
  routes: [
    {
      path: '/',
      name: 'home',
      component: Home
    },
    {
      path: '/categories',
      name: 'categories',
      component: Categories
    },
    {
      path: '/wallpaper/:id', // 动态路由，:id是壁纸的标识符
      name: 'wallpaper',
      component: Wallpaper
    },
    {
      path: '/search',
      name: 'search',
      component: Search
    },
    {
      path: '/upload',
      name: 'upload',
      component: Upload
    },
    {
      path: '/profile',
      name: 'profile',
      component: Profile
    },
    {
      path: '/login',
      name: 'login',
      component: Login
    },
    {
      path: '/anenst',
      name: 'anenst',
      component: Anenst
    },
    {
      path: '/forum',
      name: 'forum',
      component: Forum
    },
    {
      path: '/leave',
      name: 'leave',
      component: Leave
    },
    {
      path: '/register',
      name: 'register',
      component: Register
    },
    // 通常还会有一个404页面，用于匹配未定义的路由
    {
      path: '*',
      redirect: '/'
    }
  ]
});




function getImageUrls() {
  // 获取所有具有特定类的div元素
  const elements = document.querySelectorAll('.v-image__image');

  console.log(elements)
  // 用于存储背景图片url的数组
  const urls = [];

  // 遍历元素，提取style属性中的background-image
  elements.forEach(element => {
    const style = element.style;
    console.log(element)
    if (style && style.backgroundImage) {
      // 提取url(...)中的URL
      const matches = style.backgroundImage.match(/url\\("?(.+?)"?\\)/i);
      if (matches && matches[1]) {
        // 将找到的URL添加到urls数组中
        urls.push(matches[1]);
      }
    }
  });

  return urls;
}

console.log(getImageUrls());