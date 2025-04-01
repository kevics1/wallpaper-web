<template>
  <el-menu class="navbar" theme="dark" mode="horizontal" @mouseover="handleCategoryMouseOver"
    @mouseleave="handleCategoryMouseLeave">
    <el-menu-item index="logo" style="float: left;">
      <el-image :src="require('@/assets/logo-czh.png')"
        style="width: 100px; height: 100px; margin-top: -25px;"></el-image>
    </el-menu-item>

    <el-menu-item index="home" style="float: left;" class="text-zoom-effect no-underline"
      @click.native="goToPage('home')">
      首页
    </el-menu-item>

    <el-menu-item index="category" style="float: left;" class="text-zoom-effect no-underline"
      @click.native="goToPage('categories')">
      分类
    </el-menu-item>

    <el-submenu index="category" style="float: left;" class="text-zoom-effect">
      <template slot="title">
        <span class="category-title">分辨率</span>
        <!-- <i class="el-icon-caret-bottom" v-if="isCategoryHovered"></i> -->
      </template>
      <el-menu-item-group>
        <!-- 遍历一级分类 -->
        <el-submenu v-for="(category, index) in menuData.categories" :key="index" :index="category.name" ref="submenu">
          <template slot="title">
            <span @mouseover="showSubMenu(index)" @mouseleave="hideSubMenu">{{ category.name }}</span>
            <i class="el-icon-caret-bottom" v-show="activeSubMenu === index"></i>
          </template>
          <el-menu-item-group>
            <!-- 遍历二级分类 -->
            <el-menu-item v-for="(child, childIndex) in category.children" :key="childIndex">

              <router-link to="/categories" class="router-link">{{ child.name }}</router-link>
            </el-menu-item>
          </el-menu-item-group>
        </el-submenu>
      </el-menu-item-group>
    </el-submenu>

    <el-menu-item style="float: left;">
      <el-input ref="searchInput" v-model="searchQuery" placeholder="搜索壁纸..." clearable @clear="resetSearchQuery"
        suffix-icon="el-icon-search" @click.native="onSearchIconClick"></el-input>
    </el-menu-item>


    <el-menu-item index="login" @click="loginModal" style="float: right;"
      v-if="!menuData.currentUser || Object.keys(menuData.currentUser).length === 0">
      <el-button type="text" class="login-button">登录</el-button>
    </el-menu-item>

    <el-menu-item index="profile" style="float: right;"
      v-if="menuData.currentUser && Object.keys(menuData.currentUser).length !== 0">
      <el-dropdown trigger="click" placement="bottom-end">
        <div class="user-avatar" @click.stop="handleDropdownClick">
          <el-avatar :src="menuData.currentUser.avatar"></el-avatar>
          <i class="el-icon-caret-bottom"></i>
        </div>
        <el-dropdown-menu slot="dropdown">
          <el-dropdown-item>个人信息</el-dropdown-item>
          <el-dropdown-item>我的点赞</el-dropdown-item>
          <el-dropdown-item>我的收藏</el-dropdown-item>
          <el-dropdown-item @click.native="goToPage('upload')">我的上传</el-dropdown-item>
          <el-dropdown-item @click.native="handleLogout" divided>退出登录</el-dropdown-item>
        </el-dropdown-menu>
      </el-dropdown>
    </el-menu-item>
    <router-link to="/anenst" class="router-link">
      <el-menu-item index="anenst" style="float: right;" class="text-zoom-effect no-underline">
        关于
      </el-menu-item>
    </router-link>
    <router-link to="/leave" class="router-link">
      <el-menu-item index="leave" style="float: right;" class="text-zoom-effect no-underline">
        留言板
      </el-menu-item>
    </router-link>
    <router-link to="/forum" class="router-link">
      <el-menu-item index="forum" style="float: right;" class="text-zoom-effect no-underline">论坛
      </el-menu-item></router-link>
  </el-menu>
</template>

<script>
import { showLoginModal } from '@/utils/LoginModal';
import { EventBus } from '@/utils/event-bus.js';

export default {

  data() {
    return {
      menuData: {
        categories: [
          {
            "name": "手机",
            "children": [
              { "name": "1280 × 720" },
              { "name": "1920 × 1080" },
              { "name": "2560 × 1440" },
              { "name": "3840 × 2160" }
            ]
          },
          {
            "name": "电脑",
            "children": [
              { "name": "1280 × 720" },
              { "name": "1920 × 1200" },
              { "name": "2560 × 1600" },
            ]
          }
        ],
        currentUser: {}
      },
      searchQuery: '',
      activeSubMenu: -1, // 记录当前激活的子菜单索引
    };
  },
  created() {
    EventBus.$on('login-success', userData => {
      this.menuData.currentUser = userData;
    });
  },
  mounted() {
    this.loadUserData();
  },
  methods: {
    loadUserData() {
      let userData = localStorage.getItem('userData');
      if (userData) {
        userData = JSON.parse(userData);
        this.menuData.currentUser = userData;
      }
    },
    handleLogout() {
      localStorage.removeItem('userData'); // 在退出登录时清除本地存储的用户信息
      localStorage.removeItem('satoken');
      this.menuData.currentUser = {}; // 重置用户信息
    },
    goToPage(pageName) {
      if (this.$route.name !== pageName) {
        this.$router.push({ name: pageName });
      }
    },
    showSubMenu(index) {
      this.activeSubMenu = index;
      // 使用 $refs 获取当前 el-submenu 实例并手动设置 open 属性为 true
      this.$refs.submenu[index].$el.open = true;
    },
    hideSubMenu() {
      this.activeSubMenu = -1;
      // 清空所有子菜单的 open 属性
      this.$refs.submenu.forEach((submenu, index) => {
        submenu.$el.open = false;
      });
    },
    onSearchIconClick(event) {
      if (!this.searchQuery) {
        return;
      }
      const isSearchPage = this.$route.name === 'search'; // 假设 'Search' 是搜索组件的路由名称
    const currentSearchQuery = this.$route.query.searchQuery;
    
    if (!isSearchPage) {
      // 当前不在搜索页面，跳转到搜索页面并携带输入内容
      this.$router.push({ name: 'search', query: { searchQuery: this.searchQuery } });
    } else if (currentSearchQuery !== this.searchQuery) {
      // 如果已经在搜索页面，但查询参数有变化，更新参数
      this.$router.replace({ name: 'search', query: { searchQuery: this.searchQuery } });
    }
    // 如果已经在搜索页面且查询参数没有变化，则不执行任何操作
  },
    resetSearchQuery() {
      this.searchQuery = '';
    },
    handleDropdownClick() {
      // 处理下拉菜单点击事件
      console.log('点击了下拉菜单');
      // 这里添加你的下拉菜单点击逻辑
    },
    loginModal() {
      showLoginModal();
    }
  },
};
</script>

<style scoped>
.no-underline {
  text-decoration: none !important;
}

.router-link {
  text-decoration: none !important;
}

.text-zoom-effect:hover {
  transform: scale(1.3);
  transition: transform 0.3s ease;
}

.login-button {
  background-color: #0BE1FD;
  /* 设置背景颜色 */
  color: white;
  /* 设置文字颜色 */
  border-radius: 50%;
  /* 设置为圆形 */
  padding: 15px 10px;
  /* 上下内边距10px，左右内边距20px */
  border: none;
  /* 去除边框 */
  font-weight: bold;
  /* 字体加粗 */
  box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);
  /* 可选：添加阴影效果 */
  transition: background-color 0.3s;
  /* 背景颜色变化的过渡效果 */
}

.login-button:hover {
  background-color: #0ACFEC;
  /* 鼠标悬停时的背景颜色 */
}
</style>