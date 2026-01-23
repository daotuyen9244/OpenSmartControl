/**
 * NavigationBar.js
 * Bottom navigation bar of the application
 * 
 * Features:
 * - Display main navigation tabs
 * - Highlight current tab
 * - Disable control tab when not connected
 * - Responsive design
 */

import React from 'react';
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  Dimensions,
  Platform,
} from 'react-native';
import { useSelector, useDispatch } from 'react-redux';
import { navigateTo } from '../store/navigationSlice';
import { COLORS } from '../styles/Colors';
import { SCREENS } from '../utils/constants';

const { width } = Dimensions.get('window');

const NavigationBar = () => {
  const dispatch = useDispatch();
  const { currentScreen } = useSelector(state => state.navigation);
  const { isConnected } = useSelector(state => state.ble);

  // Configure navigation items
  const navItems = [
    {
      key: SCREENS.HOME,
      title: 'Home',
      icon: '🏠',
    },
    {
      key: SCREENS.QR_SCANNER,
      title: 'Scan QR',
      icon: '📱',
    },
    {
      key: SCREENS.MANUAL_CONNECT,
      title: 'Connect',
      icon: '🔗',
    },
    {
      key: SCREENS.CONTROL,
      title: 'Control',
      icon: '🎮',
      // Allow access even without connection
    },
  ];

  /**
   * Handle navigation tab press
   * @param {string} screen - Screen name
   * @param {boolean} disabled - Disabled state
   */
  const handleNavPress = (screen, disabled) => {
    if (!disabled && currentScreen !== screen) {
      dispatch(navigateTo(screen));
    }
  };

  return (
    <View style={styles.container}>
      {navItems.map((item) => (
        <TouchableOpacity
          key={item.key}
          style={[
            styles.navItem,
            currentScreen === item.key && styles.activeNavItem,
            item.disabled && styles.disabledNavItem
          ]}
          onPress={() => handleNavPress(item.key, item.disabled)}
          activeOpacity={0.7}
        >
          {/* Icon */}
          <Text style={[
            styles.navIcon,
            currentScreen === item.key && styles.activeNavIcon,
            item.disabled && styles.disabledNavIcon
          ]}>
            {item.icon}
          </Text>
          
          {/* Title */}
          <Text style={[
            styles.navText,
            currentScreen === item.key && styles.activeNavText,
            item.disabled && styles.disabledNavText
          ]}>
            {item.title}
          </Text>
          
          {/* Active tab indicator */}
          {currentScreen === item.key && <View style={styles.activeIndicator} />}
        </TouchableOpacity>
      ))}
    </View>
  );
};

// Styles for component

const styles = StyleSheet.create({
  container: {
    flexDirection: 'row',
    backgroundColor: COLORS.WHITE,
    borderTopWidth: 1,
    borderTopColor: COLORS.GRAY_LIGHT,
    paddingVertical: 8,
    paddingBottom: Platform.OS === 'ios' ? 24 : 12, // Extra padding for iOS home indicator
    elevation: 8,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: -2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  navItem: {
    flex: 1,
    alignItems: 'center',
    paddingVertical: 8,
    position: 'relative',
  },
  activeNavItem: {
    backgroundColor: 'rgba(0, 122, 255, 0.1)',
    borderRadius: 8,
    marginHorizontal: 4,
  },
  disabledNavItem: {
    opacity: 0.4,
  },
  navIcon: {
    fontSize: 20,
    marginBottom: 4,
  },
  activeNavIcon: {
    fontSize: 22,
  },
  disabledNavIcon: {
    opacity: 0.5,
  },
  navText: {
    fontSize: 10,
    color: COLORS.TEXT_SECONDARY,
    fontWeight: '500',
  },
  activeNavText: {
    color: COLORS.PRIMARY,
    fontWeight: 'bold',
    fontSize: 11,
  },
  disabledNavText: {
    color: COLORS.GRAY_MEDIUM,
  },
  activeIndicator: {
    position: 'absolute',
    top: 0,
    width: 20,
    height: 2,
    backgroundColor: COLORS.PRIMARY,
    borderRadius: 1,
  },
});

export default NavigationBar;
