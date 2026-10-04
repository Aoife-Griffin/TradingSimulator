import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react' 
import path from 'path'

export default defineConfig({
  plugins: [react()],
  server: {
    port: 3000, 
    proxy: {
      /// Maps dashboard fetch logic to local build folder
      '/dashboard_live_state.json': {
        target: 'file:///' + path.resolve(__dirname, '../build/Release/dashboard_live_state.json').replace(/\\/g, '/'),
        rewrite: (p) => p,
        changeOrigin: true
      }
    }
  }
})
