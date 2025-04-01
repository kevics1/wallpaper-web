<!-- FullImageModal.vue -->
<template>
  <el-dialog :fullscreen="false" :visible.sync="visible" :width="modalWidth" :height="modalHeight" custom-class="custom-modal" @closed="onClosed">
    <div class="modal-content">
      <img :src="imageUrl" class="modal-image" alt="Full Image" />
    </div>
  </el-dialog>
</template>

<script>
export default {
  props: {
    imageUrl: String,
  },
  data() {
    return {
      visible: true,
      modalWidth: '90vw', // 设置宽度为视窗宽度的90%
      modalHeight: '80vh', // 设置高度为视窗高度的80%
    };
  },
  methods: {
    onClosed() {
      this.$emit('close');
    },
  },
};
</script>

<style scoped lang="scss">
.custom-modal {
  background-color: transparent; 
}

.modal-content {
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100%;
  width: 100%;
  padding: 0; /* 去除模态框内部的默认填充 */
  box-sizing: border-box; /* 让padding和border计算在盒子总尺寸内 */
  background-color: transparent; /* 如果需要的话，去除背景颜色 */
}

.modal-image {
  max-width: 100%;
  max-height: 100%;
  object-fit: contain;
  border: none;
  margin: 0; /* 去除图片的外边距 */
  padding: 0; /* 去除图片的内边距 */
  box-sizing: border-box; /* 让padding和border计算在盒子总尺寸内 */
} 
</style>