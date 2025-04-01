<template>
  <div class="developing-container">
    <div class="animation-container">
      <svg class="cogwheel" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100">
        <circle cx="50" cy="50" r="47" stroke="currentColor" stroke-width="3" fill="none" />
        <g>
          <path d="M50 20l-5.5 10 5.5 0 5.5-10z" />
          <path d="M50 20l5.5-10-5.5 0-5.5 10z" />
          <animateTransform attributeName="transform" attributeType="XML" type="rotate" from="0 50 50" to="360 50 50" dur="5s" repeatCount="indefinite"/>
        </g>
        <g>
          <path d="M50 80l-5.5-10 5.5 0 5.5 10z" />
          <path d="M50 80l5.5 10-5.5 0-5.5-10z" />
        </g>
        <animateTransform attributeName="transform" attributeType="XML" type="rotate" from="0 50 50" to="-360 50 50" dur="7s" repeatCount="indefinite"/>
      </svg>
    </div>
    <h2>小菜玩命加班研发中，敬请期待...</h2>
  </div>
</template>

<script>
export default {
  name: "DevelopingPage",
};
</script>

<style scoped>
.developing-container {
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  height: 100vh;
  text-align: center;
}

.animation-container {
  margin-bottom: 20px;
}

.cogwheel {
  height: 100px;
  fill: #67C23A;
  color: #67C23A;
}

h2 {
  color: #67C23A;
}
</style>