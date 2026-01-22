const { getDefaultConfig, mergeConfig } = require('@react-native/metro-config');

const config = {
  resolver: {
    alias: {
      '@': './src',
      '@components': './src/components',
      '@services': './src/services',
      '@store': './src/store',
      '@styles': './src/styles',
    },
  },
};

module.exports = mergeConfig(getDefaultConfig(__dirname), config);
