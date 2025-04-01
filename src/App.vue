<template>
  <div id="app" @contextmenu.prevent="onContextMenu">
    <!-- Header, navigation, etc. can go here -->
    <Navbar class="navbar" />
    <!-- Router-view for displaying matched components -->
    <router-view class="router-view"></router-view>

    <!-- Footer, etc. can go here -->
  </div>
</template>

<script>
// You can also import other components and use them in this file
import Navbar from './components/Navbar'
export default {
  name: 'App',
  components: {
    Navbar
  },
  methods: {
    onContextMenu(event) {
      event.preventDefault();
      // 可选：显示自定义的右键菜单或提示信息
      // alert('右键操作已被禁用，不允许下载资源！');
    },
  }
}
</script>

<style lang="scss">
body {
  margin: 0;
  height: 100vh;
  display: flex;
  flex-direction: column;
}
#app {
  height: 100%;
  display: flex;
  flex-direction: column;
  .router-view {
    flex: 1;
  }
}
</style>