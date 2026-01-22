#!/usr/bin/env ruby
# Script to add Hermes dSYM build phase to Xcode project

require 'xcodeproj'

project_path = File.join(__dir__, '..', 'MassageChairControl.xcodeproj')
project = Xcodeproj::Project.open(project_path)

target = project.targets.find { |t| t.name == 'MassageChairControl' }
unless target
  puts "Error: Target 'MassageChairControl' not found"
  exit 1
end

# Check if build phase already exists
existing_phase = target.shell_script_build_phases.find { |phase| phase.name == 'Copy Hermes dSYM' }
if existing_phase
  puts "Build phase 'Copy Hermes dSYM' already exists"
  exit 0
end

# Create new shell script build phase
build_phase = target.new_shell_script_build_phase('Copy Hermes dSYM')
build_phase.shell_script = "\"${SRCROOT}/../scripts/copy-hermes-dsym.sh\"\n"
build_phase.run_only_for_deployment_postprocessing = 0
build_phase.show_env_vars_in_log = 0

# Add build phase before "Bundle React Native code and images" phase
# Find the React Native bundle phase
bundle_phase = target.shell_script_build_phases.find { |phase| phase.name == 'Bundle React Native code and images' }
if bundle_phase
  # Insert before bundle phase
  bundle_index = target.build_phases.index(bundle_phase)
  target.build_phases.delete(build_phase)
  target.build_phases.insert(bundle_index, build_phase)
end

project.save
puts "Successfully added 'Copy Hermes dSYM' build phase to target 'MassageChairControl'"

