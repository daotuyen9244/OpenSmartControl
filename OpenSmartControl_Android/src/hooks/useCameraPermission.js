/**
 * useCameraPermission.js
 * Custom hook to manage camera access permission
 * 
 * Features:
 * - Check current camera permission
 * - Request camera permission from user
 * - Track permission status
 * - Handle error cases
 */

import { useState, useEffect } from 'react';
import CameraPermissionService from '../services/CameraPermissionService';
import { RESULTS } from 'react-native-permissions';

/**
 * Custom hook to manage camera permission
 * @returns {Object} Object containing state and methods to manage permission
 */
export const useCameraPermission = () => {
  // State to store camera permission status
  const [hasPermission, setHasPermission] = useState(false);
  const [permissionStatus, setPermissionStatus] = useState(null);
  const [isLoading, setIsLoading] = useState(true);

  /**
   * Effect: Check camera permission when hook is initialized
   */
  useEffect(() => {
    checkPermission();
  }, []);

  /**
   * Check current camera permission
   */
  const checkPermission = async () => {
    setIsLoading(true);
    try {
      const status = await CameraPermissionService.checkCameraPermission();
      setPermissionStatus(status);
      setHasPermission(status === RESULTS.GRANTED);
    } catch (error) {
      console.error('Check permission error:', error);
      setHasPermission(false);
    } finally {
      setIsLoading(false);
    }
  };

  /**
   * Request camera permission from user
   * @returns {boolean} True if permission granted, false if denied
   */
  const requestPermission = async () => {
    try {
      const hasAccess = await CameraPermissionService.ensureCameraPermission();
      setHasPermission(hasAccess);
      
      if (!hasAccess) {
        const status = await CameraPermissionService.checkCameraPermission();
        setPermissionStatus(status);
      }
      
      return hasAccess;
    } catch (error) {
      console.error('Request permission error:', error);
      return false;
    }
  };

  // Return object containing state and methods
  return {
    hasPermission,      // Boolean: whether camera permission is granted
    permissionStatus,   // String: detailed permission status
    isLoading,         // Boolean: checking permission
    checkPermission,   // Function: check permission again
    requestPermission, // Function: request permission
  };
};

export default useCameraPermission;
